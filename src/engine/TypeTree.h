#pragma once
#include "ReyEngine.h"
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
   template <typename T>
   using MakeNodeReturnType = std::pair<std::shared_ptr<T>, std::unique_ptr<TypeNode>>;

   class TypeNode;
   struct TreeStorable {
      static constexpr char TYPE_NAME[] = "TreeCallable";
      virtual void _process(R_FLOAT delta_ms){};
      virtual void __init(){}; //added to tree for first time
      virtual void __on_made(){}; // when the node is built. A constructor of sorts, but the associated Node is valid.
      virtual void __on_added_to_tree(){}; //every time when added to tree
      virtual void __on_removed_from_tree(){}; //every time when removed from the tree
      virtual void __on_child_added_to_tree(TypeNode *n) {}; // direct children only
      virtual void __on_descendant_added_to_tree(TypeNode *n) {};
      virtual void __on_child_removed_from_tree(TypeNode*){}; // direct children only
      virtual void __on_descendant_removed_from_tree(TypeNode*){};
      virtual void __on_ancestor_removed_from_tree(TypeNode*){};
      virtual void __on_orphaned(TypeNode*){}; //when the node has been orphaned (ancestor removed from tree)
      TypeNode* getNode(){return _node;}
      [[nodiscard]] const TypeNode* getNode() const {return _node;}
   protected:
      TreeStorable(){} //cant create directly
      TreeStorable(const TreeStorable& other) = delete; //nocopy
      TreeStorable& operator=(const TreeStorable& other) = delete; //nocopy
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
      [[nodiscard]] virtual std::string getTypeName() const = 0;
   };

   //Wrappable types interface
   template<typename T>
   concept NamedType = std::is_base_of_v<TreeStorable, T> && requires {
      { T::TYPE_NAME } -> std::convertible_to<const char*>;
      requires std::is_array_v<decltype(T::TYPE_NAME)> &&
               std::is_same_v<std::remove_extent_t<decltype(T::TYPE_NAME)>, const char>;
   };

   //Type tagging interface - nodes can inherit from typetags to extend their functionality
   // this is useful to avoid the diamond problem. TypeTags CANNOT inherit from TreeStorable,
   // so one single object type can serve as the base for being stored in the tree,
   // with all additional functionality resulting from typetags.
   // Node data can be cast to base storables OR applicable typetags
   struct TypeTag{
   protected:
      TypeNode* selfNode; //allows TypeTag system to communicate with underlying TypeNode system. Typetags MUST inherit
                           // from TreeStorable, so this should always be valid. It is assigned in make_node.
      template<typename T, typename InstanceName, typename... Args>
      friend MakeNodeReturnType<T> _make_node(InstanceName&&, Args&&...);
      template<typename U, typename InstanceName, typename... Args>
      friend MakeNodeReturnType<U> _make_node(InstanceName&&, Args&&...);
   };
   template<typename T>
   concept TypeTagged = std::is_base_of_v<TypeTag, T> && !std::is_base_of_v<T, TreeStorable>;

   // Type erase wrapper. T must inherit from TreeCallable
   template<NamedType T>
   class TypeWrapper : public TypeBase {
   protected:
      TypeWrapper(T* ptr): _value(ptr){}
      template <typename... Args>
      explicit TypeWrapper(Args&&... args)
      : _value(std::forward<Args>(args)...)
      {}
   public:

      [[nodiscard]] std::type_index getTypeIndex() const override {
         return std::type_index(typeid(T));
      }

      [[nodiscard]] std::string getTypeName() const override {
         return T::TYPE_NAME;
      }

      T& getValue() { return *_value; }
      [[nodiscard]] const T& getValue() const { return *_value; }
   private:
      std::unique_ptr<T> _value;

      // Declare make_node as a friend
      template<typename U, typename InstanceName, typename... Args>
      friend MakeNodeReturnType<U> _make_node(InstanceName&&, Args&&...);
   };

   // Convert type-erased data to a format that can be stored in the tree
   struct ProtectedFunctionAccessor;
   class TypeNode {
   protected:
      explicit TypeNode(std::unique_ptr<TypeBase> data, const std::string& instanceName, const std::string& typeName)
      : _data(std::move(data))
      , name(instanceName)
      , typeName(typeName)
      {
         //link the type with the node
         as<TreeStorable>().value()->_node = this;
         _scenePath = "\\" + name;
      }
   public:
      virtual ~TypeNode(){
         if (_data) {
            Logger::debug() << "Deleting node " << name << " of type " << _data->getTypeName() << std::endl;
         }
      }
      [[nodiscard]] TypeBase* getData() const { return _data.get(); }
      inline TypeNode* getParent(){return _parent;}
      inline const TypeNode* getParent() const {return _parent;}
      inline TypeNode* getRoot(){return _root;}
      inline const TypeNode* getRoot() const {return _root;}
      inline std::string getName() const {return name;}
      inline std::string getScenePath() const {return _scenePath;}
      // Add child with a name for lookup
      TypeNode* addChild(std::unique_ptr<TypeNode>&& child) {
         if (this == nullptr){
            Logger::error() << "Null parent!";
            return nullptr;
         }
         if (!child){
            Logger::error() << "Null child cannot be added to " << name << std::endl;
            return nullptr;
         }
         if (child.get() == this){
            Logger::error() << "Child " << name << " cannot be added to itself " << name << std::endl;
            return nullptr; //child still valid at this point
         }
         const auto name = child->name;
         HashId nameHash = std::hash<std::string>{}(name);
         auto[it, success] = _childMap.try_emplace(nameHash, std::move(child));
         if (!success) {
             // Handle duplicate name case if needed
            Logger::error() << "Child " << name << " already exists for parent " << name << std::endl;
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
         // this order is really important, particularly for layouts.
         // it means that
         as<TreeStorable>().value()->__on_child_added_to_tree(childptr);
         addedStorable->__on_added_to_tree();
         auto ancestor = this;
         while (ancestor){
            //call on all that child's descendents
            std::function<void(TypeNode*)> callOnDesc = [&](TypeNode* currentNode){
               ancestor->as<TreeStorable>().value()->__on_descendant_added_to_tree(currentNode);
               for (auto& child : currentNode->getChildren()){
                  callOnDesc(child);
               }
            };
            callOnDesc(childptr);
            ancestor = ancestor->_parent;
         }
         //init, if needed
         if (!addedStorable->_has_inited) addedStorable->__init();
         return childptr;
      }

      //call necessary callbacks
      void cleanupOnRemoval(){
         //when a node is removed from the tree, we have to call a bunch of callbacks

         //notify ancestors that we were removed
         {
            auto ancestor = getParent();
            while (ancestor) {
               ancestor->as<TreeStorable>().value()->__on_descendant_removed_from_tree(this);
               ancestor = ancestor->_parent;
            }
         }
         //notify descendents that we were removed
         for (auto& descendent : getChildren()) {
            std::function<void(TypeNode *)> callOnDescendent = [&](TypeNode* removedNode) {
               descendent->as<TreeStorable>().value()->__on_ancestor_removed_from_tree(removedNode);
               //descendent must do its own cleanup too at this point too since it is also being removed from the tree
               descendent->cleanupOnRemoval();
            };
            callOnDescendent(this);
         }

         //call our own cleanup
         as<TreeStorable>().value()->__on_removed_from_tree();
      }

      /**
       * removes the node with the given name from the _childMap
       * @param name : name of the wanted node
       * @return : the unique_ptr of the node if it existed, nullptr if it did not;
       */
      std::optional<std::unique_ptr<TypeNode>> removeChild (const std::string& name){
         //remove first from child order
         //remove also from the order vector
         for (auto it = _childOrder.begin(); it != _childOrder.end(); ++it){
            auto& ptr = *it;
            if (ptr->name == name){
               _childOrder.erase(it);
               break;
            }
         }

         auto it = _childMap.find(std::hash<std::string>{}(name));
         if(it != _childMap.end()){
            auto removedNode = std::move(it->second);
            _childMap.erase(it);
            removedNode->cleanupOnRemoval();
            return removedNode;
         }
         Logger::error() << "Unable to remove child " << name << " from node " << getScenePath() << std::endl;
         return {};
      };

      std::vector<std::unique_ptr<TypeNode>> removeAllChildren (){
         std::vector<std::unique_ptr<TypeNode>> retVector;
         for(auto it = _childMap.begin(); it != _childMap.end(); it++){
            retVector.push_back(std::move(it->second));
         }
         _childMap.clear();
         _childOrder.clear();
         return retVector;
      };


      inline std::optional<TypeNode*> getChild(const std::string& name) {
         auto it = _childMap.find(std::hash<std::string>{}(name));
         if (it != _childMap.end()) return {it->second.get()};
         return std::nullopt;
      }

      TypeNode& getChildByIndex(size_t index) {
         return *_childOrder.at(index);
      }

      template<typename T>
      std::shared_ptr<T> ref() {
         if (_data->getTypeIndex() == std::type_index(typeid(T))) {
            // First, cast to TypeWrapper<T>
            auto wrapper = std::dynamic_pointer_cast<TypeWrapper<T>>(_data);
            if (wrapper) {
               // Then return a shared_ptr to the contained value
               return std::shared_ptr<T>(&wrapper->getValue(), [wrapper](T*){});
            }
         }
         return nullptr;
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
         if (!_data) return {};
         if (auto wrapper = dynamic_cast<TypeWrapper<T>*>(_data.get())) {
            return &wrapper->getValue();
         }
         // T must store a named type
         auto* wrapper = static_cast<TypeWrapper<TreeStorable>*>(_data.get());
         if (auto* value = dynamic_cast<T*>(&wrapper->getValue())) {
            return value;
         }
         return std::nullopt;
      }

      template<NamedType T>
      [[nodiscard]] std::optional<const T*> tag() const{
         return (const_cast<TypeNode*>(this))->as<T>();
      }

      template<TypeTagged T> std::optional<T*> tag() {
         if (_data) {
            if (auto storable = as<TreeStorable>()){
               if (auto v = dynamic_cast<T*>(storable.value())) return v;
            }
         }
         return {};
      }
      template<TypeTagged T> [[nodiscard]] std::optional<const T*> tag() const {return const_cast<TypeNode*>(this)->as<T>();}

      [[nodiscard]] const std::vector<TypeNode*>& getChildren() const {return _childOrder;}
      std::vector<TypeNode*>& getChildren() {return _childOrder;}

      const std::string name;
      const std::string typeName;
   protected:

      //direct, unsafe type conversion
      template<typename T>
      T* dangerousIs() {
         return &static_cast<TypeWrapper<T>*>(_data.get())->getValue();
      }

   private:
      std::string _scenePath;
      TypeNode* _parent = nullptr;
      TypeNode* _root = this;
      std::shared_ptr<TypeBase> _data;
      std::map<NameHash, std::unique_ptr<TypeNode>> _childMap; //parents own children
      std::vector<TypeNode*> _childOrder;         // Points to map entries


      // Make make_node a friend so it can access the protected constructor
      template<typename T, typename InstanceName, typename... Args>
      friend MakeNodeReturnType<T> _make_node(InstanceName&&, Args&&...);

      friend struct ReyEngine::Internal::Tree::ProtectedFunctionAccessor;
   };

   //use this if you're damn sure you know what you're doing
   struct ProtectedFunctionAccessor{
      ProtectedFunctionAccessor(TypeNode* node)
      : protectee(node)
      {}
      template <typename T>T* dangerousIs(){return protectee->dangerousIs<T>();}
      TypeNode* protectee;
      friend class TypeNode;
   };

   // Primary template for when arguments are provided
   template<typename T, typename InstanceName, typename... Args>
   MakeNodeReturnType<T> _make_node(InstanceName&& instanceName, Args&&... args) {
      auto ptr = new T(std::forward<Args>(args)...);
      auto wrapper = std::unique_ptr<TypeWrapper<T>>(new TypeWrapper<T>(ptr));
      auto node = std::unique_ptr<TypeNode>(new TypeNode(std::move(wrapper), instanceName, T::TYPE_NAME));
      //assign typetag self if applicable
      if (auto isTagged = node->tag<TypeTag>()){
         isTagged.value()->selfNode = node.get();
      }
      node->as<TreeStorable>().value()->__on_made();
      return {node->ref<T>(), std::move(node)};
   }

   //secondary template with move semantics
   template<typename T, typename InstanceName>
   MakeNodeReturnType<T> _make_node(InstanceName&& instanceName, std::unique_ptr<T>&& ptr) {
      auto wrapper = std::make_unique<TypeWrapper<T>>(ptr);
      auto node = std::unique_ptr<TypeNode>(new TypeNode(std::move(wrapper), instanceName, T::TYPE_NAME));
      //assign typetag self if applicable
      if (auto isTagged = node->tag<TypeTag>()){
         isTagged.value()->selfNode = node.get();
      }
      node->as<TreeStorable>().value()->__on_made();
      return {node->ref<T>(), std::move(node)};
   }
}
