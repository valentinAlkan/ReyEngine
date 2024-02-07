#include "Inspector.h"
#include "Button.hpp"
#include "InspectorWidgetFactory.h"

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
void Inspector::_init() {
   //add some fields
   mainVLayout = make_shared<VLayout>("MainVLayout", Rect<int>(0,0,100,100));
   addChild(mainVLayout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Inspector::inspect(std::shared_ptr<BaseWidget> widget) {
   mainVLayout->removeAllChildren();
   mainVLayout->setSize(getSize());
//   for (const auto& property : widget->getProperties()){
//      Application::printDebug() << property.first << " = " << property.second->toString() << endl;
//      auto widget = InspectorWidgetFactory::generateWidget(*property.second);
//      widget->setName(widget->getName() + "_" + property.first);
//      mainVLayout->addChild(widget);
//   }

}
