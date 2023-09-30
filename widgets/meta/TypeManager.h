#pragma once

#include "BaseWidget.h"
#include <functional>
#include <map>

using Deserializer = std::function<std::shared_ptr<BaseWidget>(const std::string& typeName, const std::string& instanceName, PropertyPrototypeMap&)>;
using Serializer = std::function<std::string(std::shared_ptr<BaseWidget>)>;

class TypeManager{
public:
   static void registerType(std::string typeName, Serializer, Deserializer);
   static std::shared_ptr<BaseWidget> deserialize(const std::string& typeName, const std::string& instanceName, PropertyPrototypeMap&);
   static std::string serialize(std::shared_ptr<BaseWidget>);
protected:
   void _registerTypes();
   static TypeManager& instance(){static TypeManager instance;return instance;}
   friend class Application;
private:
   TypeManager(){}
   void registerSerializer(std::string typeName, Serializer);
   void registerDeserializer(std::string typeName, Deserializer);
   std::map<std::string, Serializer> _serializers;
   std::map<std::string, Deserializer> _deserializers;
public:
   TypeManager(TypeManager const&) = delete;
   void operator=(TypeManager const&) = delete;
};
