#include  <stdexcept>
#include "Property.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void BaseProperty::_load(const PropertyPrototype &data) {
   //Initialize ourselves and our subproperties
   load(data);
   for (auto& [name, subproperty] : _properties){
      //load subproperties
      subproperty->_load(data.subproperties.at(name));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void PropertyContainer::registerProperty(BaseProperty& property) {
   auto found = _properties.find(property.instanceName());
   if (found != _properties.end()){
      throw runtime_error("Property instance " + property.instanceName() + " already registered to container");
   }
   //adds property if it doesn't exist
   updateProperty(property);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PropertyContainer::moveProperty(std::shared_ptr<BaseProperty> property){
   auto found = _ownedProperties.find(property->instanceName());
   if (found != _ownedProperties.end()){
      throw std::runtime_error("Property " + property->instanceName() + " already exists!");
   }
   _ownedProperties[property->instanceName()] = property;
   registerProperty(*property);

}

/////////////////////////////////////////////////////////////////////////////////////////
void PropertyContainer::updateProperty(BaseProperty& property) {
   _properties[property.instanceName()] = &property;
   property.registerProperties();

   //register any properties this property contains
   for (auto& [name, subprop] : property._properties){
      subprop->registerProperties();
   }
}