#include "InspectorWidgetFactory.h"
#include "Layout.h"
#include "Label.hpp"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> InspectorWidgetFactory::generateWidget(const BaseProperty& property) {
   std::shared_ptr<BaseWidget> retval = VLayout::build<VLayout>("__vLayout");
   if (property.typeName() == PropertyTypes::Rect){
      auto hbox = HLayout::build<HLayout>("__hLayout");
      retval->addChild(hbox);
      auto nameLabel = Label::build<Label>("__nameLabel");
      nameLabel->setText(property.instanceName());
      hbox->addChild(nameLabel);
      auto valueLabel = Label::build<Label>("__valueLabel");
      valueLabel->setText(property.toString());
      hbox->addChild(valueLabel);
   }
    return retval;
}