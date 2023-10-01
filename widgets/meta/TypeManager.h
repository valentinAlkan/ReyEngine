#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <optional>
#include "Property.h"

class BaseWidget;
using Deserializer = std::function<std::shared_ptr<BaseWidget>(const std::string& instanceName, PropertyPrototypeMap&)>;

struct TypeMeta {
   TypeMeta(std::string typeName, std::string parent, bool isVirtual, Deserializer deserializer)
   : typeName(typeName)
   , parent(parent)
   , isVirtual(isVirtual)
   , deserializer(deserializer)
   {}
   std::string typeName;
   Deserializer deserializer;
   std::string parent;
   std::map<std::string, std::shared_ptr<TypeMeta>> children;
   bool isVirtual; //whether or not it can be instanced
};

class TypeManager{
public:
   static void registerType(std::string typeName, std::string parentType, bool isVirtual, Deserializer);
   static std::shared_ptr<BaseWidget> deserialize(const std::string& typeName, const std::string& instanceName, PropertyPrototypeMap&);
   std::shared_ptr<TypeMeta> getType(std::string typeName);
   std::shared_ptr<TypeMeta> getRoot(){return getType("BaseWidget");};
protected:
   void _registerTypes();
   static TypeManager& instance(){static TypeManager instance;return instance;}
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
