#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <optional>
#include "Property.h"

namespace ReyEngine::Internal {
   class Component;
   using Deserializer = std::function<std::shared_ptr<Component>(const std::string& instanceName, PropertyPrototypeMap&)>;

   struct TypeMeta {
      TypeMeta(std::string typeName, std::string parentTypeName, bool isVirtual, Deserializer deserializer)
      : typeName(typeName)
      , deserializer(deserializer)
      , parentTypeName(parentTypeName)
      , isVirtual(isVirtual)
      {}
      std::string typeName;
      Deserializer deserializer;
      std::string parentTypeName;
      std::map<std::string, std::shared_ptr<TypeMeta>> children;
      bool isVirtual; //whether or not it can be instanced
   };

   class TypeManager{
   public:
      static void registerType(std::string typeName, std::string parentTypeName, bool isVirtual, Deserializer);
      static std::shared_ptr<Component> deserialize(const std::string& typeName, const std::string& instanceName, PropertyPrototypeMap&);
      static std::shared_ptr<TypeMeta> getType(std::string typeName);
      std::shared_ptr<TypeMeta> getRoot(){return getType("Component");};
      static TypeManager& instance(){static TypeManager instance;return instance;}
      void _registerTypes();
   protected:
      friend class Application;
   private:
      TypeManager(){}
      std::map<std::string, std::shared_ptr<TypeMeta>> _types;
      std::weak_ptr<TypeMeta> root; //reference to type stored in map;
      std::shared_ptr<TypeMeta> getTypeCanBeNull(std::string typeName);
   public:
      TypeManager(TypeManager const&) = delete;
      void operator=(TypeManager const&) = delete;
      friend class Application;
   };
}