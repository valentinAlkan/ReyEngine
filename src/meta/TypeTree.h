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
#include "Logger.h"

namespace ReyEngine::Internal::Tree {
   using HashId = size_t;
   using NameHash = HashId;

   class TypeNode;
   struct TreeStorable {
      static constexpr char TYPE_NAME[] = "TreeCallable";
      virtual void __init(){}; //added to tree for first time
      virtual void __on_added_to_tree(){}; //every time when added to tree
      virtual void __on_child_added_to_tree(TypeNode *n) {}; // direct children only
      virtual void __on_descendant_added_to_tree(TypeNode *n) {};
      virtual void __on_child_removed_from_tree(TypeNode*){}; // direct children only
      virtual void __on_descendant_removed_from_tree(TypeNode*){};
      virtual void __on_orphaned(TypeNode*){}; //when the node has been orphaned (ancestor removed from tree)
      TypeNode* getNode(){return _node;}
      [[nodiscard]] const TypeNode* getNode() const {return _node;}
   protected:
      TypeNode* _node = nullptr;
      bool _has_inited = false;
   private:
      friend class TypeNode;
   };


   // Type-erased base class for TypeWrapper
   class TypeBase {
   public:
      virtual ~TypeBase() = default;
      [[nodiscard]] virtual std::type_index getTypeIndex() const = 0;
   };

   //Wrappable types interface
   template<typename T>
   concept NamedType = std::is_base_of_v<TreeStorable, T> && requires(T t) {
      { T::TYPE_NAME } -> std::convertible_to<const char*>;
      requires std::is_array_v<decltype(T::TYPE_NAME)> && std::is_same_v<std::remove_extent_t<decltype(T::TYPE_NAME)>, const char>;
   };

   struct TypeTag{};
   //Type tagging interface
   template<typename T>
   concept TypeTagged = std::is_base_of_v<TypeTag, T> && !std::is_base_of_v<TreeStorable, T>;

   // Type erase wrapper. T must inherit from TreeCallable
   template<NamedType T>
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
   private:
      T _value;
   };

   // Convert type-erased data to a format that can be stored in the tree
   class TypeNode {
   public:
      explicit TypeNode(TypeBase* data, const std::string& instanceName, const std::string& typeName)
      : _data(data)
      , name(instanceName)
      , typeName(typeName)
      {
         //link the type with the node
         as<TreeStorable>().value()->_node = this;
         _scenePath = "\\" + name;
      }
      virtual ~TypeNode(){
         std::cout << "Deleting type node " << name << " of type " << name << std::endl;
      }
      [[nodiscard]] TypeBase* getData() const { return _data.get(); }
      inline TypeNode* getParent(){return _parent;}
      inline TypeNode* getRoot(){return _root;}
      inline std::string getScenePath(){return _scenePath;}
      // Add child with a name for lookup
      TypeNode* addChild(std::unique_ptr<TypeNode>&& child) {
         const auto& name = child->name;
         HashId nameHash = std::hash<std::string>{}(name);
         auto[it, success] = _childMap.emplace(nameHash, std::move(child));
         if (!success) {
             // Handle duplicate name case if needed
            Logger::error() << "Child " << name << " already exists for parent " << name;
            return nullptr; //child still valid at this point
         }
         auto childptr = it->second.get();
         auto addedStorable = childptr->as<TreeStorable>().value();
         // update child order vector
         _childOrder.push_back(childptr);

         // Set parent
         childptr->_parent = this;

         //set the root
         childptr->_root = _root;
         childptr->_scenePath = _scenePath + "\\" + name;

         //////callbacks
         //init, if needed
         if (!addedStorable->_has_inited) addedStorable->__init();
         addedStorable->__on_added_to_tree();
         as<TreeStorable>().value()->__on_child_added_to_tree(childptr);
         auto ancestor = this;
         while (ancestor){
            ancestor->as<TreeStorable>().value()->__on_descendant_added_to_tree(childptr);
            ancestor = ancestor->_parent;
         }
         return childptr;
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
      template<NamedType T>
      std::optional<T*> as() {
         if (auto wrapper = dynamic_cast<TypeWrapper<T>*>(_data.get())) {
            return &wrapper->getValue();
         }
         // T must store a named type
         if (_data) {
            auto* wrapper = static_cast<TypeWrapper<TreeStorable>*>(_data.get());
            if (auto* value = dynamic_cast<T*>(&wrapper->getValue())) {
               return value;
            }
         }
         return std::nullopt;
      }

      template<NamedType T>
      [[nodiscard]] std::optional<const T*> as() const{
         return (const_cast<TypeNode*>(this))->as<T>();
      }

      template<TypeTagged T> std::optional<T*> as() {
         if (_data) {
            if (auto storable = as<TreeStorable>()){
               if (auto v = dynamic_cast<T*>(storable.value())) return v;
            }
         }
         return {};
      }
      template<TypeTagged T> [[nodiscard]] std::optional<const T*> as() const {return const_cast<TypeNode*>(this)->as<T>();}

      [[nodiscard]] const std::vector<TypeNode*>& getChildren() const {return _childOrder;}
      std::vector<TypeNode*>& getChildren() {return _childOrder;}

//      struct DuplicateNameError : public std::runtime_error {explicit DuplicateNameError(const std::string& message) : std::runtime_error(message) {}};
//      struct BadTypeError : public std::runtime_error {explicit BadTypeError(const std::string& message) : std::runtime_error(message) {}};
      const std::string name;
      const std::string typeName;
   private:
      std::string _scenePath;
      TypeNode* _parent = nullptr;
      TypeNode* _root = this;
      std::shared_ptr<TypeBase> _data;
      std::map<NameHash, std::unique_ptr<TypeNode>> _childMap; //parents own children
      std::vector<TypeNode*> _childOrder;         // Points to map entries
   };

   template<typename T, typename InstanceName, typename... Args>
   requires std::constructible_from<T, Args...>
   static std::unique_ptr<TypeNode> make_node(InstanceName&& instanceName, Args&&... args) {
      return std::unique_ptr<TypeNode>(new TypeNode(new TypeWrapper<T>(std::forward<Args>(args)...), instanceName, T::TYPE_NAME));
   }
}
