#include "Component.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;
using namespace FileSystem;
using namespace Internal;

///////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const std::string &name, const std::string& typeName)
: TypeContainer<Component>(name, typeName)
, NamedInstance(name, typeName)
, _resourceId("resourceId", Application::getNewRid())
, _isProcessed("isProcessed")
, _name(name)
, _typeName(typeName)
{}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Component::serialize() {
   stringstream data;
   data << _name << " - " << _typeName << ":\n";
   for (const auto& [name, property] : _properties){
      auto value = property->toString();
      data << PropertyMeta::INDENT << property->instanceName();
      data << PropertyMeta::SEP << property->typeName(); //todo: ? pretty sure this isn't actually necesary
      data << PropertyMeta::SEP << value.size();
      data << PropertyMeta ::SEP << value;
      data << ";";
   }
   return data.str();
}

///////////////////////////////////////////////////////////////////////////////////////////
uint64_t Component::getFrameCounter() const {
   return Application::instance().getWindow(0).getFrameCounter();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Component::_deserialize(PropertyPrototypeMap& propertyData){
   //register all properties so we know what's what
   _register_parent_properties();
   registerProperties();
   //   move the properties to their new home
   for (auto& [name, data] : propertyData){
      auto found = _properties.find(name);
      if (found == _properties.end()){
         throw std::runtime_error("Property " + name + " of type " + data.typeName + " not registered to type " + _typeName + ". Did you remember to call ParentType::registerProperties() for each parent type?");
         //dynamic property - throw it in the data struct

      }
      found->second->_load(data);
   }

}
/////////////////////////////////////////////////////////////////////////////////////////
template <>
void ReyEngine::Internal::TypeContainer<Component>::__on_child_added_immediate(std::shared_ptr<Component>& child){
   child->__init();
   _on_child_added(child);
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Component> Component::toComponent() {
   return inheritable_enable_shared_from_this<Component>::downcasted_shared_from_this<Component>();
}