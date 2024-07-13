#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include "Logger.h"
#include "Event.h"
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
         EVENT_GENERATE_UNIQUE_ID(ChildAddedEvent)
         EVENT_GET_NAME(ChildAddedEvent)
         explicit ChildAddedEvent(std::shared_ptr<EventPublisher> publisher, ChildPtr& child)
         : Event<ChildAddedEvent>(ChildAddedEvent_UNIQUE_ID, publisher)
         , child(child)
         {}
         ChildPtr& child;
      };
      struct DescendentAddedEvent : public Event<DescendentAddedEvent> {
         EVENT_GENERATE_UNIQUE_ID(DescendentAddedEvent)
         EVENT_GET_NAME(DescendentAddedEvent)
         explicit DescendentAddedEvent(std::shared_ptr<EventPublisher> publisher, ChildPtr& descendent)
         : Event<DescendentAddedEvent>(DescendentAddedEvent_UNIQUE_ID, publisher)
         , descendent(descendent)
         {}
         ChildPtr& descendent;
      };

      struct DescendentRemovedEvent : public Event<DescendentRemovedEvent> {
         EVENT_GENERATE_UNIQUE_ID(DescendentRemovedEvent)
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
      virtual void addChild(ChildPtr child){
//         static_assert(std::is_base_of_v<T, decltype(*child)>);
         auto me = toContainedTypePtr();
         auto& parent = me;
         if (child == parent){
            std::stringstream ss;
            ss << "Cannot add widget " << child->getName() << " to itself!" << std::endl;
            Logger::error() << ss.str();
            throw std::runtime_error(ss.str());
         }
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         auto found = getChild(child->getName());
         if (found){
            std::stringstream ss;
            ss << "Container " << parent->getName() << " already has a child with name <" << child->getName() << ">";
            Logger::error() << ss.str();
            throw std::runtime_error(ss.str());
         }
         if (child->getParent().lock()){
            std::stringstream ss;
            ss << "Widget " << child->getName() << " already has a parent! It needs to be removed from its existing parent first!";
            Logger::error() << ss.str();
            throw std::runtime_error(ss.str());
         }
         //call your template specializations here
         if (std::is_same_v<Component, T>){
            ___on_component_added_immediate(child);
         }
         __on_child_added_immediate(child);
         _childOrder.push_back(child);
         _childMap[child->getName()];
      }
      std::optional<ChildPtr>removeChild(ChildPtr& child, bool quiet){
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         auto found = getChildMap().find(getName());
         if (found == getChildMap().end()){
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
               parent->_on_descendent_about_to_be_removed(child);
               parent = parent->getParent().lock();
            }
         }
         __on_exit_tree(child, true);

         auto orderIndex = found->second.first;
         getChildMap().erase(found);
         _childOrder.erase(_childOrder.begin() + orderIndex);
         child->isInLayout = false;
         _on_child_removed(child);


         //do this again
         {
            auto parent = me;
            while (parent) {
               DescendentRemovedEvent event(toEventPublisher(), child);
               parent->publish(event);
               parent->_on_descendent_removed(child);
               parent = parent->getParent().lock();
            }
         }
         __on_exit_tree(child, false);
         return child;
      }
      
      inline void removeAllChildren() {
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         //todo: put this somewhere else
//         for (auto& child : _childMap){
//            child.second.second->isInLayout = false;
//         }
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
      inline ChildOrder& getChildren() {return _childOrder;}
      inline const ChildOrder& getChildren() const {return _childOrder;}
      inline bool hasChild(const std::string& name){
         //cant be const because it locks
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         return _childMap.find(name) != _childMap.end();
      }
      inline std::vector<std::weak_ptr<T>> findChild(const std::string& name, bool exact=false){
         std::vector<std::weak_ptr<T>> retval;
         std::vector<std::weak_ptr<T>> descendents;
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         for (auto& child : getChildren()){
            for (auto& found : child->findChild(name, exact)){
               descendents.push_back(found);
            }
         }

         if (exact ? getName() == name : getName().find(name) != std::string::npos){
            retval.push_back(toContainedType());
         }
         for (auto& descendent : descendents){
            retval.push_back(descendent);
         }

         return retval;
      }

   protected:
      ChildMap& getChildMap(){return _childMap;}
      const ChildMap& getChildMap() const {return _childMap;}

      void ___on_component_added_immediate(ChildPtr&);
      virtual void _on_child_added_immediate(ChildPtr&){} //Called immediately upon a call to addChild - DANGER: type is not actually a child yet! It is (probably) a very bad idea to do much at all here. Make sure you know what you're doing.
      virtual void __on_child_added_immediate(ChildPtr&){};
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
      virtual void _on_enter_tree(){} //called EVERY TIME a type enters the tree
      virtual void __on_enter_tree(){} //called EVERY TIME a type enters the tree
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
      ChildMap _childMap;
      ChildOrder _childOrder;
   private:
      std::weak_ptr<T> _parent;
      std::recursive_mutex _childLock;
   };

//   //adapter to simply some code
//   template <typename T>
//   class TypeContainerInterface {
//   public:
//      TypeContainerInterface(TypeContainer<T>& containerRef)
//      : _containerRef(containerRef){}
//      std::optional<std::shared_ptr<T>> addchild(){return _containerRef.addChild();};
//      std::weak_ptr<T> getParent(){return _containerRef.getParent();};
//      typename TypeContainer<T>::ChildOrder& getChildren(){return _containerRef.getChildren();};
//      bool hasChild(const std::string& name){return _containerRef.hasChild(name);}
//      std::optional<std::shared_ptr<T>> getChild(const std::string& name){return _containerRef.getChild(name);}
//      std::vector<std::weak_ptr<T>> findChild(const std::string& name, bool exact){
//         std::vector<std::weak_ptr<T>> retval;
//         std::vector<std::weak_ptr<T>> descendents;
//         auto lock = std::scoped_lock<std::recursive_mutex>(_containerRef._childLock);
//         for (auto& child : getChildren()){
//            for (auto& found : child->findChild(name, exact)){
//               descendents.push_back(found);
//            }
//         }
//
//         if (exact ? getName() == name : getName().find(name) != std::string::npos){
//            retval.push_back(retval);
//         }
//         for (auto& descendent : descendents){
//            retval.push_back(descendent);
//         }
//
//         return retval;
//      }
//   protected:
//      virtual std::string getName() const = 0;
//   private:
//      TypeContainer<T>& _containerRef;
//   };
}