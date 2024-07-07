#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include "Logger.h"

namespace ReyEngine::Internal{
   template <typename T>
   class TypeContainer
   {
   public:
      using ChildIndex = unsigned long;
      using ChildPtr = std::shared_ptr<T>;
      using ChildMap = std::map<std::string, std::pair<ChildIndex, std::shared_ptr<T>>>;
      using ChildOrder = std::vector<ChildPtr>;
      TypeContainer() = default;
      T& toContainedType(){
         return (T&)(*this);
      };
      std::optional<ChildPtr> addChild(ChildPtr& parent, ChildPtr& child);
      void removeChild(ChildPtr&);
      std::optional<ChildPtr> getChild(const std::string& name){
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         auto found = _childMap.find(name);
         if (found == _childMap.end()) {
            return std::nullopt;
         }
         return {found->second.second};
      }
      std::weak_ptr<T> getParent(){return _parent;}
      ChildOrder& getChildren() {return _childOrder;}
      bool hasChild(const std::string& name){
         //cant be const because it locks
         auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
         return _childMap.find(name) != _childMap.end();
      }

   protected:
      virtual void __on_child_added(){};
      virtual void _on_child_added_immediate(ChildPtr&) = 0;
      ChildMap _childMap;
      ChildOrder _childOrder;
   private:
      std::weak_ptr<T> _parent;
      std::recursive_mutex _childLock;
   };

   //adapter to simply some code
   template <typename T>
   class TypeContainerInterface {
   public:
      TypeContainerInterface(TypeContainer<T>& containerRef)
      : _containerRef(containerRef){}
      std::optional<std::shared_ptr<T>> addchild(){return _containerRef.addChild();};
      std::weak_ptr<T> getParent(){return _containerRef.getParent();};
      typename TypeContainer<T>::ChildOrder& getChildren(){return _containerRef.getChildren();};
      bool hasChild(const std::string& name){return _containerRef.hasChild(name);}
      std::optional<std::shared_ptr<T>> getChild(const std::string& name){return _containerRef.getChild(name);}
      std::vector<std::weak_ptr<T>> findChild(const std::string& name, bool exact){
         std::vector<std::weak_ptr<T>> retval;
         std::vector<std::weak_ptr<T>> descendents;
         auto lock = std::scoped_lock<std::recursive_mutex>(_containerRef._childLock);
         for (auto& child : getChildren()){
            for (auto& found : child->findChild(name, exact)){
               descendents.push_back(found);
            }
         }

         if (exact ? getName() == name : getName().find(name) != std::string::npos){
            retval.push_back(retval);
         }
         for (auto& descendent : descendents){
            retval.push_back(descendent);
         }

         return retval;
      }
   protected:
      virtual std::string getName() const = 0;
   private:
      TypeContainer<T>& _containerRef;
   };
}