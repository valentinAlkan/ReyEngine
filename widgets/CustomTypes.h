#pragma once
#include "TestWidgets.h"

#define REGISTER_CUSTOM_TYPE(UNIQUE_CLASS_NAME, PARENT_CLASS_NAME, IS_VIRTUAL) \
BaseWidget::registerType(#UNIQUE_CLASS_NAME, #PARENT_CLASS_NAME, IS_VIRTUAL, &UNIQUE_CLASS_NAME::deserialize); \

namespace CustomTypes{
   void registerTypes(){
      /* put your custom type registration calls here */
//      BaseWidget::registerType("PosTestWidget", "BaseWidget", false, &PosTestWidget::deserialize);
      REGISTER_CUSTOM_TYPE(PosTestWidget, Control, false);

   }
}