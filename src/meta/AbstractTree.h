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
   class TypeBase {
   public:
      virtual ~TypeBase() = default;
      virtual std::type_index getTypeIndex() const = 0;
   };

// Template for concrete types
   template<typename T>
   class TypeWrapper : public TypeBase {
   public:
      explicit TypeWrapper(T value) : value_(std::move(value)) {}

      std::type_index getTypeIndex() const override {
         return std::type_index(typeid(T));
      }

      T& getValue() { return value_; }
      const T& getValue() const { return value_; }

   private:
      T value_;
   };

   // Node in the type tree using RefCounted
   class TypeNode {
   public:
      explicit TypeNode(std::unique_ptr<TypeBase> data)
      : data_(std::move(data)) {}

      TypeBase* getData() const { return data_.get(); }

      void addChild(RefCounted<TypeNode>* child) {
         children_.push_back(child);
         child->get()->parent_ = this;
      }

      template<typename T>
      T* as() {
         if (data_->getTypeIndex() == std::type_index(typeid(T))) {
            return &static_cast<TypeWrapper<T>*>(data_.get())->getValue();
         }
         return nullptr;
      }

   private:
      std::unique_ptr<TypeBase> data_;
      TypeNode* parent_ = nullptr;
      std::vector<RefCounted<TypeNode>*> children_;
   };

// Type-safe handle class using RefCounted
   class TypeHandle {
   public:
      explicit TypeHandle(RefCounted<TypeNode>* ref = nullptr) : ref_(ref) {
         if (ref_) ref_->addRef();
      }

      TypeHandle(const TypeHandle& other) : ref_(other.ref_) {
         if (ref_) ref_->addRef();
      }

      TypeHandle(TypeHandle&& other) noexcept : ref_(other.ref_) {
         other.ref_ = nullptr;
      }

      ~TypeHandle() {
         if (ref_) ref_->release();
      }

      TypeHandle& operator=(const TypeHandle& other) {
         if (this != &other) {
            if (ref_) ref_->release();
            ref_ = other.ref_;
            if (ref_) ref_->addRef();
         }
         return *this;
      }

      template<typename T>
      T* as() {
         return ref_ ? ref_->get()->as<T>() : nullptr;
      }

   private:
      RefCounted<TypeNode>* ref_;
   };

// The tree that manages type storage
   class TypeTree {
   public:
      template<typename T>
      TypeHandle addType(const std::vector<std::string>& path, T value) {
         auto node = new TypeNode(
               std::make_unique<TypeWrapper<T>>(std::move(value))
         );
         auto refCounted = new RefCounted<TypeNode>(node);

         // Store in lookup table
         size_t hash = std::hash<std::string>{}(joinPath(path));
         types_[hash] = refCounted;

         // Build tree structure
         if (path.size() > 1) {
            auto parentPath = path;
            parentPath.pop_back();
            if (auto parent = findNode(parentPath)) {
               parent->get()->addChild(refCounted);
            }
         }

         return TypeHandle(refCounted);
      }

      TypeHandle findType(const std::vector<std::string>& path) {
         if (auto node = findNode(path)) {
            return TypeHandle(node);
         }
         return TypeHandle(nullptr);
      }

      ~TypeTree() {
         // Clean up all nodes
         for (auto& [_, ref] : types_) {
            ref->release();
         }
      }

   private:
      RefCounted<TypeNode>* findNode(const std::vector<std::string>& path) {
         size_t hash = std::hash<std::string>{}(joinPath(path));
         auto it = types_.find(hash);
         return it != types_.end() ? it->second : nullptr;
      }

      static std::string joinPath(const std::vector<std::string>& path) {
         std::string result;
         for (const auto& part : path) {
            if (!result.empty()) result += "/";
            result += part;
         }
         return result;
      }

      std::map<size_t, RefCounted<TypeNode>*> types_;
   };

}
