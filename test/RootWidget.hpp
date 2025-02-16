#pragma once
#include "Control.h"

namespace ReyEngine{
   class RootWidget : public BaseWidget{
      REYENGINE_OBJECT_BUILD_ONLY(RootWidget, BaseWidget, BaseWidget){}
   public:
      REYENGINE_DEFAULT_BUILD(RootWidget)
      void render2D() const override {
         drawRectangle(getRect(), theme->background.colorPrimary.value);
      }
      void _process(float dt) override {}
      void registerProperties() override{
         //register properties specific to your type here.
      }
   };
}