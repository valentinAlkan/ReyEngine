#pragma once
#include "Control.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , renderTarget(_rect.value.size())
   {
      //create scrollbars
   }
public:
   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {
      renderTarget.beginRenderMode();
      renderTarget.clear();
      textureOffset -= getGlobalPos();
   }
   void renderEnd() override {
      renderTarget.endRenderMode();
      renderTarget.render(getGlobalPos() + getTextureRenderModeOffset());
   }
   void render() const override {
//      _drawRectangleGradientV({0, 0, (int)_rect.value.width, (int)_rect.value.height}, BLUE, RED);
   }
   void _process(float dt) override {}
   void registerProperties() override{
   }
   void _on_rect_changed() override {
      renderTarget.resize(_rect.value.size());
   }

protected:
   GFCSDraw::RenderTarget renderTarget;
   GFCSDraw::Vec2<float> scrollOfset;
};

