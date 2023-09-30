#include <stdexcept>
#include "TypeManager.h"
#include "CustomTypes.h"
#include "BaseWidget.h"
/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::registerType(std::string typeName, Deserializer fx) {
   auto found = instance()._deserializers.find(typeName);
   if (found != instance()._deserializers.end()){
      throw std::runtime_error("Typename " + typeName + " already has a registered deserializer!");
   }
   instance()._deserializers[typeName] = std::move(fx);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::_registerTypes() {
   //register all internal widget types here

   //register custom types
   CustomTypes::registerTypes();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> TypeManager::deserialize(const std::string &typeName, const std::string &instanceName, PropertyPrototypeMap& protoperties) {
   auto& instance = TypeManager::instance();
   auto found = instance._deserializers.find(typeName);
   if (found == instance._deserializers.end()){
      throw std::runtime_error("No deserializer for type " + typeName);
   }
   auto fx = found->second;
   return fx(instanceName, protoperties);
}