#pragma once
#include "ReyEngine.h"
#include "RefCounted.h"
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>

namespace ReyEngine::Internal::Tree {
   // Base class for all types that can be stored in the tree
   class NamedInstance2{
   public:
      NamedInstance2(const std::string& instancename, const std::string& typeName)
      : instanceName(instancename)
      , typeName(typeName)
      {}
      const std::string instanceName;
      const std::string typeName;
   };

   // virtual base for type-erasure
   class TypeBase {
   public:
      virtual ~TypeBase() = default;
      virtual std::type_index getTypeIndex() const = 0;
   };

   // Type erase wrapper
   template<typename T>
   class TypeWrapper : public TypeBase {
   public:
      explicit TypeWrapper(T value)
      : value_(std::move(value))
      {}

      [[nodiscard]] std::type_index getTypeIndex() const override {
         return std::type_index(typeid(T));
      }

      T& getValue() { return value_; }
      const T& getValue() const { return value_; }

   private:
      T value_;
   };

   // Convert type-erased data to a format that can be stored in the tree
   class TypeNode {
   public:
      TypeNode(TypeNode&& other)
      : instanceInfo(other.instanceInfo)
      , _data(std::move(other._data))
      {}
      explicit TypeNode(TypeBase* data, const std::string& instanceName, const std::string& typeName)
      : instanceInfo(instanceName, typeName)
      , _data(std::unique_ptr<TypeBase>(data)) {}
      virtual ~TypeNode(){
         std::cout << "Goodbye from typeNode " << instanceInfo.instanceName << " of type " << instanceInfo.instanceName << std::endl;
      }
      [[nodiscard]] TypeBase* getData() const { return _data.get(); }

      // Add child with a name for lookup
      inline void addChild(TypeNode&& child) {
         auto rc = make_ref_counted<TypeNode>(std::move(child));
         addChild(rc);
      }
      inline void addChild(RefCounted<TypeNode>& child) {
         const auto& name = child->instanceInfo.instanceName;
         size_t nameHash = std::hash<std::string>{}(name);

         // create ref count and store it in the map
         auto [it, inserted] = _childMap.emplace(nameHash, child);
         if (!inserted) {
            // Handle duplicate name case if needed
            throw DuplicateNameError("Node " + name);
         }

         // Store pointer to map's RefCounted in order vector
         _childOrder.push_back(&it->second);

         // Set parent
         it->second->_parent = this;
      }

      inline std::optional<RefCounted<TypeNode>> getChild(const std::string& name) {
         auto it = _childMap.find(std::hash<std::string>{}(name));
         if (it != _childMap.end()) return {it->second};
         return std::nullopt;
      }

      RefCounted<TypeNode>& getChildByIndex(size_t index) {
         return *_childOrder.at(index);
      }
      template<typename T>
      std::optional<T*> as() {
         if (_data->getTypeIndex() == std::type_index(typeid(T))) {
            return &static_cast<TypeWrapper<T>*>(_data.get())->getValue();
         }
         return {};
      }
      struct DuplicateNameError : public std::runtime_error {explicit DuplicateNameError(const std::string& message) : std::runtime_error(message) {}};
      const NamedInstance2 instanceInfo;
   private:
      TypeNode* _parent = nullptr;
      std::unique_ptr<TypeBase> _data;
      std::map<size_t, RefCounted<TypeNode>> _childMap;       // map of types
      std::vector<RefCounted<TypeNode>*> _childOrder;         // Points to map entries
   };


   template<typename T, typename First, typename Second, typename... Args>
   static TypeNode make_node(First&& instanceName, Second&& typeName, Args&&... args) {
      auto* wrap = new TypeWrapper<T>(T(std::forward<Args>(args)...));
      return TypeNode(wrap, instanceName, typeName);
   }

   template<typename T, typename First, typename Second, typename... Args>
   static RefCounted<TypeNode> make_node_refcounted(First&& instanceName, Second&& typeName, Args&&... args) {
      auto* wrap = new TypeWrapper<T>(T(std::forward<Args>(args)...));
      return make_ref_counted<TypeNode>(wrap, instanceName, typeName);
   }
}
