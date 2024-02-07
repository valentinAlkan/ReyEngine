#include "InspectorWidgetFactory.h"
#include "Layout.hpp"
#include "Label.hpp"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> InspectorWidgetFactory::generateWidget(const BaseProperty& property) {
   auto r = Rect<int>();
   std::shared_ptr<BaseWidget> retval = make_shared<VLayout>("__vLayout", r);
   if (property.typeName() == PropertyTypes::Rect){
      auto hbox = make_shared<HLayout>("__hLayout", r);
      retval->addChild(hbox);
      auto nameLabel = make_shared<Label>("__nameLabel", r);
      nameLabel->setText(property.instanceName());
      hbox->addChild(nameLabel);
      auto valueLabel = make_shared<Label>("__valueLabel", r);
      valueLabel->setText(property.toString());
      hbox->addChild(valueLabel);
   }
    return retval;
}