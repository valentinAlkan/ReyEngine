#pragma once
#include "Control.hpp"
#include "Slider.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , scrollOffsetX(0, 100, 0)
   , scrollOffsetY(0, 100, 0)
   {}
public:
   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {
//      renderTarget.beginRenderMode();
//      renderTarget.clear();
//      textureOffset -= getGlobalPos();
      scissorTarget.start(getGlobalRect());
      //draw test under children
      auto offsetRange = getScrollAreaSize();
      _drawRectangleGradientV({0, 0, (int)offsetRange.x, (int)offsetRange.y}, BLACK, BROWN);
   }
   void renderEnd() override {
//      renderTarget.endRenderMode();
//      renderTarget.render(getGlobalPos() + getTextureRenderModeOffset());
//      GFCSDraw::drawTextCentered(scrollOffset.toString(), _rect.get().toSizeRect().center(), 20, RED);
      scissorTarget.stop();
   }
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{
   }
   void _on_rect_changed() override {
      auto newSize = getRect().size();
      scrollOffsetX.setRange(0, newSize.x);
      scrollOffsetY.setRange(0, newSize.y);
   }
   inline GFCSDraw::Vec2<double> getScrollAreaSize() const {return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};}
   inline GFCSDraw::Vec2<double> getScrollOffset() const {return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};}

protected:
   void _init() override {
      //create scrollbars
      auto scrollSize = _rect.get().size();
      auto sliderSize = 20;
      auto vslider = std::make_shared<Slider>("__vslider", GFCSDraw::Rect<float>{(float)(scrollSize.x-sliderSize), 0, (float)sliderSize, (float)scrollSize.y}, Slider::SliderType::VERTICAL);
      auto hslider = std::make_shared<Slider>("__hslider", GFCSDraw::Rect<float>{0, (float)(scrollSize.y - sliderSize), (float)(scrollSize.x - sliderSize), (float)sliderSize}, Slider::SliderType::HORIZONTAL);
      addChild(vslider);
      addChild(hslider);
      std::cout << "scrollOffsetX = " << scrollOffsetX << std::endl;

      auto echo = [&](){
         std::cout << "scrollOffsetX = " << scrollOffsetX << std::endl;
         std::cout << "scrollOffsetY = " << scrollOffsetY << std::endl;
      };

      auto setOffsetX = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         scrollOffsetX.setLerp(event.pct);
         std::cout << "scrollOffsetX = " << scrollOffsetX << std::endl;
         std::cout << "scrollOffsetY = " << scrollOffsetY << std::endl;
//         echo();
      };
      auto setOffsetY = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         scrollOffsetY.setLerp(event.pct);
         std::cout << "scrollOffsetX = " << scrollOffsetX << std::endl;
         std::cout << "scrollOffsetY = " << scrollOffsetY << std::endl;
//         echo();
      };
      subscribe<Slider::SliderValueChangedEvent>(hslider, setOffsetX);
      subscribe<Slider::SliderValueChangedEvent>(vslider, setOffsetY);
   }
//   GFCSDraw::RenderTarget renderTarget;
   GFCSDraw::ScissorTarget scissorTarget;
   GFCSDraw::Range<double> scrollOffsetX;
   GFCSDraw::Range<double> scrollOffsetY;
};

