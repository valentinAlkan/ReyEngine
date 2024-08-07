#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include "Logger.h"
#include "Event.h"

#define FIXME(identifier) throw std::runtime_error("FIXME: " #identifier)
namespace ReyEngine::Internal{
    class Component;
    class NamedInstance
    : public EventPublisher
    , public EventSubscriber
    {
    public:
        NamedInstance(const std::string& instanceName, const std::string& typeName)
        : _name(instanceName)
        , _type(typeName)
        {}
        static constexpr char TYPE_NAME[] = "NamedType";
        virtual std::string _get_static_constexpr_typename() {return TYPE_NAME;}
        inline std::string getName() const {return _name;}
        inline std::string getType() const {return _type;}
    private:
        std::string _name;
        std::string _type;
    };

    template <typename T>
    class TypeContainer
    : public virtual NamedInstance
    , public inheritable_enable_shared_from_this<TypeContainer<T>>
    {
    public:
        using ChildIndex = unsigned long;
        using ChildPtr = std::shared_ptr<T>;
        using ChildMap = std::map<std::string, std::pair<ChildIndex, std::shared_ptr<T>>>;
        using ChildOrder = std::vector<ChildPtr>;

        struct ChildAddedEvent : public Event<ChildAddedEvent> {
            EVENT_GENERATE_UNIQUE_ID(ChildAddedEvent, Event_ChildAddedEvent)
            EVENT_GET_NAME(ChildAddedEvent)
            explicit ChildAddedEvent(std::shared_ptr<EventPublisher> publisher, ChildPtr& child)
            : Event<ChildAddedEvent>(ChildAddedEvent_UNIQUE_ID, publisher)
            , child(child)
            {}
            ChildPtr& child;
        };
        struct DescendentAddedEvent : public Event<DescendentAddedEvent> {
            EVENT_GENERATE_UNIQUE_ID(DescendentAddedEvent, Event_DescendentAddedEvent)
            EVENT_GET_NAME(DescendentAddedEvent)
            explicit DescendentAddedEvent(std::shared_ptr<EventPublisher> publisher, ChildPtr& descendent)
            : Event<DescendentAddedEvent>(DescendentAddedEvent_UNIQUE_ID, publisher)
            , descendent(descendent)
            {}
            ChildPtr& descendent;
        };

        struct DescendentRemovedEvent : public Event<DescendentRemovedEvent> {
            EVENT_GENERATE_UNIQUE_ID(DescendentRemovedEvent, Event_DescendentRemovedEvent)
            EVENT_GET_NAME(DescendentRemovedEvent)
            explicit DescendentRemovedEvent(std::shared_ptr<EventPublisher> publisher, ChildPtr& descendent)
            : Event<DescendentRemovedEvent>(DescendentRemovedEvent_UNIQUE_ID, publisher)
            , descendent(descendent)
            {}
            ChildPtr& descendent;
        };


        TypeContainer(const std::string& instanceName, const std::string& typeName)
        : NamedInstance(instanceName, typeName)
        {}
        T& toContainedType(){
            return (T&)(*this);
        };
        ChildPtr toContainedTypePtr(){
            return inheritable_enable_shared_from_this<TypeContainer<T>>::template downcasted_shared_from_this<T>();
        };
        void addChild(std::shared_ptr<TypeContainer<T>> child){
            auto me = toContainedTypePtr();
            auto childTypePtr = child->toContainedTypePtr();
            if (child == me){
                std::stringstream ss;
                ss << "Cannot add widget " << child->getName() << " to itself!" << std::endl;
                Logger::error() << ss.str();
                throw std::runtime_error(ss.str());
            }
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            auto found = getChild(child->getName());
            if (found){
                std::stringstream ss;
                ss << "Container " << me->getName() << " already has a child with name <" << child->getName() << ">";
                Logger::error() << ss.str();
                throw std::runtime_error(ss.str());
            }
            if (child->getParent().lock()){
                std::stringstream ss;
                ss << "Widget " << child->getName() << " already has a parent! It needs to be removed from its existing parent first!";
                Logger::error() << ss.str();
                throw std::runtime_error(ss.str());
            }
            //set the parent
            child->_parent = me;
            Logger::debug() << "Registering child " << child->getName() << " to parent " << getName() << std::endl;

            auto newIndex = getChildren().size(); //index of new child's location in ordered vector
            _childOrder.push_back(childTypePtr);
            _childMap[getName()] = std::pair<int, ChildPtr>(newIndex, me);
            __on_child_added_immediate(childTypePtr);
            if (isInTree()){
               child->doEnterTree();
            }
        }

        std::optional<ChildPtr>removeChild(const std::string& name, bool quiet=false){
            auto found = findChild(name);
            if (found){
               removeChild(found.value(), quiet);
            } else if(!quiet) {
               std::stringstream ss;
               ss << getType() << "::" << getName() << " does not have a child with name <" << name << ">";
               Logger::error() << ss.str() << std::endl;
            }
            return std::nullopt;
        }

        std::optional<ChildPtr>removeChild(ChildPtr& child, bool quiet){
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            auto found = _childMap.find(child->getName());
            if (found == _childMap.end()){
                if (!quiet) {
                    std::stringstream ss;
                    ss << getType() << "::" << getName() << " does not have a child with name <" << getName() << ">";
                    Logger::error() << ss.str() << std::endl;
                }
                return std::nullopt;
            }

            auto me = toContainedTypePtr();
            {
                auto parent = me;
                while (parent) {
                    DescendentRemovedEvent event(toEventPublisher(), child);
                    parent->publish(event);
                    parent->TypeContainer<T>::_on_descendent_about_to_be_removed(child);
                    parent = parent->getParent().lock();
                }
            }
            __on_exit_tree(child, true);

            auto orderIndex = found->second.first;
            _childMap.erase(found);
            _childOrder.erase(_childOrder.begin() + orderIndex);
            _on_child_removed(child);

            //do this again
            {
                auto parent = me;
                while (parent) {
                    DescendentRemovedEvent event(toEventPublisher(), child);
                    parent->publish(event);
                    parent->TypeContainer<T>::_on_descendent_removed(child);
                    parent = parent->getParent().lock();
                }
            }
            __on_exit_tree(child, false);
            return child;
        }

        inline void removeAllChildren() {
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            _childOrder.clear();
            _childMap.clear();
        }

        inline std::optional<ChildPtr> getChild(const std::string& name){
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            auto found = _childMap.find(name);
            if (found == _childMap.end()) {
                return std::nullopt;
            }
            return {found->second.second};
        }
        std::weak_ptr<T> getParent(){return _parent;}
        const std::weak_ptr<T> getParent() const {return _parent;}
        inline ChildOrder& getChildren() {return _childOrder;}
        inline const ChildOrder& getChildren() const {return _childOrder;}
        inline bool hasChild(const std::string& name){
            //cant be const because it locks
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            return _childMap.find(name) != _childMap.end();
        }

        inline std::optional<ChildPtr>findChild(const std::string& name){
           auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
           for (auto& child : getChildren()){
              if (child->getName() == name){
                 return child;
              }
           }
           return std::nullopt;
        }

        inline std::vector<std::weak_ptr<T>> findDescendents(const std::string& name, bool exact=false){
            std::vector<std::weak_ptr<T>> retval;
            std::vector<std::weak_ptr<T>> descendents;
            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
            for (auto& child : getChildren()){
                for (auto& found : child->TypeContainer<T>::findDescendents(name, exact)){
                    descendents.push_back(found);
                }
            }

            if (exact ? getName() == name : getName().find(name) != std::string::npos){
                retval.push_back(toContainedTypePtr());
            }
            for (auto& descendent : descendents){
                retval.push_back(descendent);
            }

            return retval;
        }


       /////////////////////////////////////////////////////////////////////////////////////////
        bool setName(const std::string& newName, bool append_index) {
           FIXME(TypeContainer::setName);
//            auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
//            //if the child has a sibling by the same name, it cannot be renamed
//            if (!_parent.expired()) {
//                //has a parent
//                auto self = toBaseWidget();
//                string _newName = newName;
//                auto parent = _parent.lock();
//                if (parent->getChild(newName)) {
//                    //parent has existing child with that name
//                    //if we are allowed to, just append an index to the name (start at 2)
//                    if (append_index) {
//                        int index = 2;
//                        while (parent->getChild(newName + to_string(index))) {
//                            index++;
//                        }
//                        //todo: is this right?
//                        _newName = newName + to_string(index);
//                    } else {
//                        return false;
//                    }
//                }
//                //parent does not have a child with the name
//                parent->rename(self, _newName);
//            }
//            //root widget or orphaned, no need to deal with parent
//            _name = newName;
//            return true;
        }

        void rename(ChildPtr &child, const std::string &newName) {
           FIXME(TypeContainer::setName);
            //rename but not move
            //find the existing reference to the child
//    auto& children = getChildren();
//    auto childIter = children[child->_name];
//    auto oldName = child->_name;
//    _children[newName] = childIter;
//    _children.erase(oldName);
//    child->_name = newName;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        bool setIndex(unsigned int newIndex){
            if (_parent.expired()){
                //root widget, there is no index
                return false;
            }
            auto parent = _parent.lock();
            //get reference to widget in map
            FIXME(TypeContainer::setIndex);
//    auto selfMapIter = parent->_children[_name];
//    auto index = selfMapIter.first;
//
//    //delete reference to widget in ordered vector
//    auto selfVectorIter = parent->_childrenOrdered.begin() + index;
//    parent->_childrenOrdered.erase(selfVectorIter);
//
//    //insert new reference in new position
//    //todo: finish
//    throw std::runtime_error("not finished");
//
//

        }
        bool isRoot() {return _isRoot;}
        bool isInTree(){
           if (TypeContainer<T>::_isRoot){
               return true;
            }
            auto parent = getParent().lock();
            if (!parent) return false;
            return parent->TypeContainer<T>::isInTree();
        }
    protected:
        void setRoot(bool isRoot){_isRoot = isRoot;}
        const ChildMap& getChildMap() const {return _childMap;}
        virtual void _on_child_added_immediate(ChildPtr&){} //Called immediately upon a call to addChild - DANGER: type is not actually a child yet! It is (probably) a very bad idea to do much at all here. Make sure you know what you're doing.
        void __on_child_added_immediate(ChildPtr&);
        virtual void _on_child_added(ChildPtr&){} // called at the beginning of the next frame after a child is added. Child is now owned by us. Safe to manipulate child. Called after all events are emitted.
        virtual void __on_child_added(ChildPtr&){};
        virtual void _on_descendent_added(ChildPtr&){} // All parents up the chain will emit this signal. Emits along with _on_child_added when this node is the parent.
        virtual void __on_descendent_added(ChildPtr&){}; // Internal.
        virtual void _on_child_removed(ChildPtr&){}
        virtual void __on_child_removed(ChildPtr&){}
        virtual void _on_descendent_removed(ChildPtr&){} // Internal
        virtual void __on_descendent_removed(ChildPtr&){} // Internal
        virtual void _on_descendent_about_to_be_removed(ChildPtr&){} // All parents up the chain will emit this signal. Emits along with _on_child_removed when this node is the parent.
        virtual void __on_descendent_about_to_be_removed(ChildPtr&){}
       //called EVERY TIME a type enters the tree
        virtual void _on_enter_tree(){}
        virtual void __on_enter_tree(){}
        virtual void _on_exit_tree(ChildPtr&, bool aboutToExit){};
        virtual void __on_exit_tree(ChildPtr&, bool aboutToExit){};
        virtual void _on_about_to_exit_tree(){} //called right before a type leave
        virtual void __on_about_to_exit_tree(){};
        virtual void _on_exit_tree(){} //called right after a type leaves the tree
        virtual void __on_exit_tree(){};
        void __on_exit_tree(bool aboutToExit){
            //Every descendent of the removed widget must call it's own exit tree functions.
            // This will occurr in reverse order, starting with the most descendent child and ending with the one that was removed.
            auto me = toContainedTypePtr();
            for (auto it = _childOrder.rbegin(); it != _childOrder.rend(); it++) {
                auto &child = *it;
                child->__on_exit_tree(me, true);
            }
            //so we can use this function for either version
            if (aboutToExit){
                _on_about_to_exit_tree();
            } else {
                _on_exit_tree();
            }
        } //called right after a type leaves the tree
       bool _isRoot = false;
    private:
       void doEnterTree(){
           __on_enter_tree();
           _on_enter_tree();
           for (auto& child : _childOrder) {
              child->TypeContainer<T>::doEnterTree();
           }
       }
       std::weak_ptr<T> _parent;
       std::recursive_mutex _childLock;
       ChildMap _childMap;
       ChildOrder _childOrder;
    };
}