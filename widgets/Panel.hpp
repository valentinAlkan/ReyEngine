#pragma once
#include "Control.hpp"

// A simple visual panel that just gives us something to look at. No other functionality.
class Panel : public Control {
public:
   GFCSDRAW_OBJECT(Panel, Control){
      getTheme()->background.colorPrimary.set(GFCSDraw::ColorRGBA(156, 181, 156, 255));
   }
public:
   void render() const override {
      auto roundness = getThemeReadOnly().roundness.get();
      auto color = getThemeReadOnly().background.colorPrimary.get();
      _drawRectangleRounded(_rect.value.toSizeRect(), roundness, 1, color);
   }
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
};