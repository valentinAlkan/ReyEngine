#pragma once
#include "ReyEngine.h"
#include "RefCounted.h"
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <set>
#include "MapValueRefView.h"

namespace ReyEngine::Internal::Tree {
   using HashId = size_t;
   using NameHash = HashId;

   class NamedInstance2{
   public:
      NamedInstance2(const std::string& instancename, const std::string& typeName)
      : instanceName(instancename)
      , typeName(typeName)
      {}
      const std::string instanceName;
      const std::string typeName;
      std::string getScenePath(){return scenePath;}
   private:
      std::string scenePath;
   };

   // Base class for all types that can be stored in the tree
   // virtual base for type-erasure
   class TypeNode;
   class TypeBase {
   public:
      virtual ~TypeBase() = default;
      virtual std::type_index getTypeIndex() const = 0;
      TypeNode& getNode(){return *_node;};
      const TypeNode& getNode() const {return *_node;}
   protected:
      TypeNode* _node = nullptr;
      friend class TypeNode;
   };

   //Wrappable types interface
   template<typename T>
   concept TypeWrappable = requires(T t, TypeNode* node) {
      {t._on_added_to_tree(node)};
      {t._on_child_added_to_tree(node)};
      { T::TYPE_NAME } -> std::convertible_to<const char*>;
      requires std::is_array_v<decltype(T::TYPE_NAME)> && std::is_same_v<std::remove_extent_t<decltype(T::TYPE_NAME)>, const char>;
   };

   // Type erase wrapper
   template<TypeWrappable T>
   class TypeWrapper : public TypeBase {
   public:
      template <typename... Args>
      explicit TypeWrapper(Args&&... args)
      requires std::constructible_from<T, Args...>
      : _value(std::forward<Args>(args)...)
      {}

      [[nodiscard]] std::type_index getTypeIndex() const override {
         return std::type_index(typeid(T));
      }

      T& getValue() { return _value; }
      const T& getValue() const { return _value; }
   protected:

   private:
      T _value;
   };

   // Define a concept
//   class TypeNode;
//   template<typename T>
//   concept NodeType = std::is_base_of_v<T, TypeNode>;

   // Convert type-erased data to a format that can be stored in the tree
   class TypeNode {
   public:
      explicit TypeNode(TypeBase* data, const std::string& instanceName, const std::string& typeName)
      : instanceInfo(instanceName, typeName)
      , _data(data) {
         _data->_node = this;
      }
      virtual ~TypeNode(){
         std::cout << "Deleting type node " << instanceInfo.instanceName << " of type " << instanceInfo.instanceName << std::endl;
      }
      [[nodiscard]] TypeBase* getData() const { return _data.get(); }

      // Add child with a name for lookup
      template <TypeWrappable ChildType, TypeWrappable ThisType>
      void addChild(std::unique_ptr<TypeNode>&& child) {
         const auto& name = child->instanceInfo.instanceName;
         HashId nameHash = std::hash<std::string>{}(name);

         auto[it, success] = _childMap.emplace(nameHash, std::move(child));
         if (!success) {
             // Handle duplicate name case if needed
            throw DuplicateNameError("Node " + name);
         }
         auto childPtr = it->second.get();

         // update child order vector
         _childOrder.push_back(childPtr);

         // Set parent
         childPtr->_parent = this;
         //callbacks
         is<ThisType>().value()->_on_added_to_tree(this);
         childPtr->is<ChildType>().value()->_on_child_added_to_tree(childPtr);

      }

      inline std::optional<TypeNode*> getChild(const std::string& name) {
         auto it = _childMap.find(std::hash<std::string>{}(name));
         if (it != _childMap.end()) return {it->second.get()};
         return std::nullopt;
      }

      TypeNode& getChildByIndex(size_t index) {
         return *_childOrder.at(index);
      }
      template<typename T>
      std::optional<T*> is() {
         if (_data->getTypeIndex() == std::type_index(typeid(T))) {
            return &static_cast<TypeWrapper<T>*>(_data.get())->getValue();
         }
         return {};
      }
      template<typename T>
      std::optional<T*> as() {
         if (auto wrapper = dynamic_cast<TypeWrapper<T>*>(_data.get())) {
            return &wrapper->getValue();
         }
         return std::nullopt;
      }

      struct DuplicateNameError : public std::runtime_error {explicit DuplicateNameError(const std::string& message) : std::runtime_error(message) {}};
      const NamedInstance2 instanceInfo;
   private:
      TypeNode* _parent = nullptr;
      std::shared_ptr<TypeBase> _data;
      std::map<NameHash, std::unique_ptr<TypeNode>> _childMap; //parents own children
      std::vector<TypeNode*> _childOrder;         // Points to map entries
   };

   template<typename T, typename InstanceName, typename... Args>
   requires std::constructible_from<T, Args...>
   static std::unique_ptr<TypeNode> make_node(InstanceName&& instanceName, Args&&... args) {
      TypeWrapper<T>* wrap;
      {
         wrap = new TypeWrapper<T>(std::forward<Args>(args)...);
      }
      return std::unique_ptr<TypeNode>(new TypeNode(wrap, instanceName, T::TYPE_NAME));
   }
}
