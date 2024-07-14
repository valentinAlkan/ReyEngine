#include "Inspector.h"
#include "Button.h"
#include "InspectorWidgetFactory.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Inspector::_init() {
   //add some fields
   mainVLayout = VLayout::build<VLayout>("MainVLayout");
   mainVLayout->setSize({100,100});
   addChild(mainVLayout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Inspector::inspect(std::shared_ptr<BaseWidget> widget) {
   mainVLayout->removeAllChildren();
   mainVLayout->setSize(getSize());
//   for (const auto& property : widget->getProperties()){
//      Logger::debug() << property.first << " = " << property.second->toString() << endl;
//      auto widget = InspectorWidgetFactory::generateWidget(*property.second);
//      widget->setName(widget->getName() + "_" + property.first);
//      mainVLayout->addChild(widget);
//   }

}
