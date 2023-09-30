#include "TypeManager.h"

/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::registerType(std::string typeName, Serializer, Deserializer) {}

/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::_registerTypes() {
   //register all internal widget types here
}
/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::registerSerializer(std::string typeName, Serializer fx) {
   auto found = _serializers.find(typeName);
   if (found != _serializers.end()){
      throw std::runtime_error("Name already exists!");
   }
   _serializers[typeName] = std::move(fx);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::registerDeserializer(std::string typeName, Deserializer fx) {
   auto found = _deserializers.find(typeName);
   if (found != _deserializers.end()){
      throw std::runtime_error("Name already exists!");
   }
   _deserializers[typeName] = std::move(fx);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string TypeManager::serialize(std::shared_ptr<BaseWidget> widget) {
   auto& instance = TypeManager::instance();
   auto found = instance._serializers.find(widget->getTypeName());
   if (found == instance._serializers.end()){
      throw std::runtime_error("No serializer found for type " + widget->getTypeName());
   }
   auto serializer = found->second;
   return serializer(widget);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> TypeManager::deserialize(const std::string &typeName, const std::string &instanceName, PropertyPrototypeMap& protoperties) {
   auto& instance = TypeManager::instance();
   auto found = instance._deserializers.find(typeName);
   if (found == instance._deserializers.end()){
      throw std::runtime_error("No deserializer for type " + typeName);
   }
   auto fx = found->second;
   return fx(typeName, instanceName, protoperties);
}