#pragma once
#include "Control.hpp"
#include "Slider.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , renderTarget(_rect.value.size())
   {}
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
   void _init() override {
      //create scrollbars
      auto vslider = std::make_shared<Slider>("__vslider", GFCSDraw::Rect<float>{0,0,20,100}, Slider::SliderType::VERTICAL);
      auto hslider = std::make_shared<Slider>("__hslider", GFCSDraw::Rect<float>{20,20,100,20}, Slider::SliderType::HORIZONTAL);
      addChild(vslider);
      addChild(hslider);
   }
   GFCSDraw::RenderTarget renderTarget;
   GFCSDraw::Vec2<float> scrollOfset;
};

