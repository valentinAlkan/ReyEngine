#pragma once
#include "Control.hpp"

class RootWidget : public Control{
   GFCSDRAW_OBJECT(RootWidget, Control){}
public:
//   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {}
   void render() const override {
      auto& theme = refTheme();
      _drawRectangle(_rect.value, theme.background.colorPrimary.get());
   }
//   void renderEnd() override{}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
};