#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include "Property.h"

class BaseWidget;
using Deserializer = std::function<std::shared_ptr<BaseWidget>(const std::string& instanceName, PropertyPrototypeMap&)>;
using Serializer = std::function<std::string()>;

class TypeManager{
public:
   static void registerType(std::string typeName, Deserializer);
   static std::shared_ptr<BaseWidget> deserialize(const std::string& typeName, const std::string& instanceName, PropertyPrototypeMap&);
protected:
   void _registerTypes();
   static TypeManager& instance(){static TypeManager instance;return instance;}
   friend class Application;
private:
   TypeManager(){}
   std::map<std::string, Deserializer> _deserializers;
public:
   TypeManager(TypeManager const&) = delete;
   void operator=(TypeManager const&) = delete;
   friend class Application;
};
