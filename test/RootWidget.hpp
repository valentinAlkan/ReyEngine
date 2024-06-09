#pragma once
#include "Control.h"

namespace ReyEngine{
   class RootWidget : public BaseWidget{
      REYENGINE_OBJECT(RootWidget, BaseWidget){}
   public:
   //   void renderBegin(ReyEngine::Vec2<float>& textureOffset) override {}
      void render() const override {
         drawRectangle(_rect.value, theme->background.colorPrimary.value);
      }
   //   void renderEnd() override{}
      void _process(float dt) override {}
      void registerProperties() override{
         //register properties specific to your type here.
      }
   };
}