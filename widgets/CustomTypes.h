#pragma once
#include "TestWidgets.h"
namespace CustomTypes{
   void registerTypes(){
      /* put your custom type registration calls here */
      BaseWidget::registerType<PosTestWidget>("PosTestWidget", "BaseWidget", false, &PosTestWidget::deserialize);
   }
}