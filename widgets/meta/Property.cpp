#include  <stdexcept>
#include "Property.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void PropertyContainer::registerProperty(BaseProperty& property) {
   auto found = _properties.find(property.instanceName());
   if (found != _properties.end()){
      throw runtime_error("Property instance " + property.instanceName() + " already registered to container");
   }
   _properties[property.instanceName()] = &property;
}
