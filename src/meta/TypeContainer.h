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
   protected:
      void _on_child_added_immediate(ChildPtr&);
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
   private:
      TypeContainer<T>& _containerRef;
   };
}