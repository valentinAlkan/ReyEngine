#include "InspectorWidgetFactory.h"
#include "Layout.h"
#include "Label.hpp"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> InspectorWidgetFactory::generateWidget(const BaseProperty& property) {
   std::shared_ptr<BaseWidget> retval = VLayout::build("__vLayout");
   if (property.typeName() == PropertyTypes::Rect){
      auto hbox = HLayout::build("__hLayout");
      retval->addChild(hbox);
      auto nameLabel = Label::build("__nameLabel");
      nameLabel->setText(property.instanceName());
      hbox->addChild(nameLabel);
      auto valueLabel = Label::build("__valueLabel");
      valueLabel->setText(property.toString());
      hbox->addChild(valueLabel);
   }
    return retval;
}