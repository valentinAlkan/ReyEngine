#pragma once
#include "Control.hpp"
#include "Slider.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , scrollOffsetRange(GFCSDraw::Vec2<int>(0,1000))
   {}
public:
   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {
//      renderTarget.beginRenderMode();
//      renderTarget.clear();
//      textureOffset -= getGlobalPos();
      scissorTarget.start(getGlobalRect());
   }
   void renderEnd() override {
//      renderTarget.endRenderMode();
//      renderTarget.render(getGlobalPos() + getTextureRenderModeOffset());
//      GFCSDraw::drawTextCentered(scrollOffset.toString(), _rect.get().toSizeRect().center(), 20, RED);
      scissorTarget.stop();
   }
   void render() const override {
      _drawRectangleGradientV({100, 100, (int)_rect.value.width, (int)_rect.value.height}, BLUE, RED);
   }
   void _process(float dt) override {}
   void registerProperties() override{
   }
   void _on_rect_changed() override {
//      renderTarget.resize(_rect.value.size());
   }

protected:
   void _init() override {
      //create scrollbars
      auto scrollSize = _rect.get().size();
      auto sliderSize = 20;
      auto vslider = std::make_shared<Slider>("__vslider", GFCSDraw::Rect<float>{(float)(scrollSize.x-sliderSize), 0, (float)sliderSize, (float)scrollSize.y}, Slider::SliderType::VERTICAL);
      auto hslider = std::make_shared<Slider>("__hslider", GFCSDraw::Rect<float>{0, (float)(scrollSize.y-sliderSize), (float)(scrollSize.x-sliderSize),  (float)sliderSize}, Slider::SliderType::HORIZONTAL);
      addChild(vslider);
      addChild(hslider);
      auto setOffsetX = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         scrollOffset.x = (float)scrollOffsetRange.lerp(event.pct);
      };
      auto setOffsetY = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         scrollOffset.y = (float)scrollOffsetRange.lerp(event.pct);
      };
      subscribe<Slider::SliderValueChangedEvent>(hslider, setOffsetX);
      subscribe<Slider::SliderValueChangedEvent>(vslider, setOffsetY);
   }
//   GFCSDraw::RenderTarget renderTarget;
   GFCSDraw::ScissorTarget scissorTarget;
   GFCSDraw::Vec2<float> scrollOffset;
   GFCSDraw::Vec2<float> scrollOffsetRange;
};

