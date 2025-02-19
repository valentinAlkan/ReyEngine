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
   using TypeHash = size_t;

   class NamedInstance2{
   public:
      NamedInstance2(const std::string& instancename, const std::string& typeName)
      : instanceName(instancename)
      , typeName(typeName)
      {}
      const std::string instanceName;
      const std::string typeName;
   };

   // Base class for all types that can be stored in the tree
   // virtual base for type-erasure
   class TypeBase {
   public:
      virtual ~TypeBase() = default;
      virtual std::type_index getTypeIndex() const = 0;
   };

   //Wrappable types interface
   template<typename T>
   concept TypeWrappable = requires(T t) {
      {t._on_added_to_tree()};
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
      : value_(std::forward<Args>(args)...)
      {}

      [[nodiscard]] std::type_index getTypeIndex() const override {
         return std::type_index(typeid(T));
      }

      void _on_added_to_tree() {
         value_._on_added_to_tree();
      };

      T& getValue() { return value_; }
      const T& getValue() const { return value_; }


   private:
      T value_;
   };

   // Convert type-erased data to a format that can be stored in the tree
   class TypeNode {
   public:
      explicit TypeNode(TypeBase* data, const std::string& instanceName, const std::string& typeName)
      : instanceInfo(instanceName, typeName)
      , _data(RefCounted<TypeBase>(data)) {}
      virtual ~TypeNode(){
         std::cout << "Goodbye from typeNode " << instanceInfo.instanceName << " of type " << instanceInfo.instanceName << std::endl;
      }
      [[nodiscard]] TypeBase* getData() const { return _data.get(); }

      // Add child with a name for lookup
      inline void addChild(std::unique_ptr<TypeNode>&& child) {
         const auto& name = child->instanceInfo.instanceName;
         TypeHash nameHash = std::hash<std::string>{}(name);

         // create ref count and store it in the map
         auto[it, inserted] = _childMap.emplace(nameHash, std::move(child));
         if (!inserted) {
             // Handle duplicate name case if needed
            throw DuplicateNameError("Node " + name);
         }

         // Store pointer to map's RefCounted in order vector
         _childOrder.push_back(&it->second);

         // Set parent
         it->second->_parent = this;

//         //callbacks
//         child->
      }

      inline std::optional<TypeNode*> getChild(const std::string& name) {
         auto it = _childMap.find(std::hash<std::string>{}(name));
         if (it != _childMap.end()) return {it->second.get()};
         return std::nullopt;
      }

      TypeNode& getChildByIndex(size_t index) {
         return **_childOrder.at(index);
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
         return &dynamic_cast<TypeWrapper<T>*>(_data.get())->getValue();
      }

      //Tree-oriented functions
      virtual void _on_added_to_tree(){};

      struct DuplicateNameError : public std::runtime_error {explicit DuplicateNameError(const std::string& message) : std::runtime_error(message) {}};
      const NamedInstance2 instanceInfo;
   private:
      TypeNode* _parent = nullptr;
      RefCounted<TypeBase> _data;
      std::map<TypeHash, std::unique_ptr<TypeNode>> _childMap;       // map of types
      std::vector<std::unique_ptr<TypeNode>*> _childOrder;         // Points to map entries
   };

   template<typename T, typename InstanceName, typename... Args>
   requires std::constructible_from<T, Args...>
   static std::unique_ptr<TypeNode> make_node(InstanceName&& instanceName, Args&&... args) {
      auto* wrap = new TypeWrapper<T>(T(std::forward<Args>(args)...));
      return std::unique_ptr<TypeNode>(new TypeNode(wrap, instanceName, T::TYPE_NAME));
   }
}
