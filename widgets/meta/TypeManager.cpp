#include <stdexcept>
#include "TypeManager.h"
#include "CustomTypes.h"
#include "BaseWidget.h"
#include "Button.h"

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::registerType(std::string typeName, string parentType, bool isVirtual, Deserializer fx) {
   if (typeName.empty()){
      throw std::runtime_error("Typename cannot be empty!");
   }
   string msg = "Typename " + typeName +" ";
   if (parentType.empty()){
      throw std::runtime_error(msg + "parentType cannot be empty!");
   }
   if (instance().getTypeCanBeNull(typeName)){
      throw std::runtime_error(msg + "has already been registered!");
   }
   if (!isVirtual && fx == nullptr){
      throw std::runtime_error(msg + "requires a valid deserializer function!");
   }
   if (!instance().getTypeCanBeNull(parentType)){
      throw std::runtime_error(msg + "parent type " + parentType + " has not been registered!");
   }
   if (isVirtual && fx){
      throw std::runtime_error(msg + " virtual types cannot have deserializers!");
   }
   instance()._types[typeName] = make_shared<TypeMeta>(typeName, parentType, isVirtual, std::move(fx));
}

/////////////////////////////////////////////////////////////////////////////////////////
void TypeManager::_registerTypes() {
   //Manually register BaseWidget
   static constexpr char basewidget[] = "BaseWidget";
   instance()._types[basewidget] = make_shared<TypeMeta>(basewidget, "", true, nullptr);
   //register all other internal widget types here
   BaseButton::registerType("BaseButton", "BaseWidget", true, nullptr);

   //register custom types
   CustomTypes::registerTypes();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> TypeManager::deserialize(const std::string &typeName, const std::string &instanceName, PropertyPrototypeMap& protoperties) {
   auto& instance = TypeManager::instance();
   auto found = instance.getType(typeName);
   if (!found->deserializer){
      throw std::runtime_error("No deserializer for type " + typeName);
   };
   if (found->isVirtual){
      throw std::runtime_error("Type " + typeName + " is virtual and cannot be instantiated or deserialized!");
   }
   return found->deserializer(instanceName, protoperties);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<TypeMeta> TypeManager::getType(string typeName) {
   auto& instance = TypeManager::instance();
   auto found = instance._types.find(typeName);
   if (found == instance._types.end()){
      throw std::runtime_error("No TypeMeta for type " + typeName);
   }
   return found->second;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<TypeMeta> TypeManager::getTypeCanBeNull(std::string typeName) {
   auto& instance = TypeManager::instance();
   auto found = instance._types.find(typeName);
   if (found == instance._types.end()){
      return nullptr;
   }
   return found->second;
}