#pragma once
#include "Control.h"

class RootWidget : public Control{
   REYENGINE_OBJECT(RootWidget, Control){}
public:
//   void renderBegin(ReyEngine::Vec2<float>& textureOffset) override {}
   void render() const override {
      _drawRectangle(_rect.value, theme->background.colorPrimary.value);
   }
//   void renderEnd() override{}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
};