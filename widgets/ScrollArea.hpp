#pragma once
#include "Control.hpp"
#include "Slider.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , scrollOffsetX(0, getWidth(), 0)
   , scrollOffsetY(0, getHeight(), 0)
   {}
   inline GFCSDraw::Size<double> getScrollAreaSize() const {return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};}
   inline GFCSDraw::Pos<double> getScrollOffset() const {return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};}
protected:
   void renderBegin(GFCSDraw::Pos<double>& textureOffset) override {
      textureOffset -= getScrollOffset();
      scissorTarget.start(getGlobalRect());
   }
   void renderEnd() override {
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
   void _on_child_added(std::shared_ptr<BaseWidget>& child) override {
      //expand scroll size to fit children
      auto rectSize = getChildRectSize();
      scrollOffsetX.setMax(rectSize.x);
      scrollOffsetY.setMax(rectSize.y);
   }

protected:
   void _init() override {
      //create scrollbars
      auto scrollSize = _rect.get().size();
      auto sliderSize = 20;
      vslider = std::make_shared<Slider>("__vslider", GFCSDraw::Rect<float>{(float)(scrollSize.x-sliderSize), 0, (float)sliderSize, (float)scrollSize.y}, Slider::SliderType::VERTICAL);
      hslider = std::make_shared<Slider>("__hslider", GFCSDraw::Rect<float>{0, (float)(scrollSize.y - sliderSize), (float)(scrollSize.x - sliderSize), (float)sliderSize}, Slider::SliderType::HORIZONTAL);
      addChild(vslider);
      addChild(hslider);

      auto setOffsetX = [this](const Slider::SliderValueChangedEvent& event){
         scrollOffsetX.setLerp(event.pct);
         hslider->setRenderOffset(getScrollOffset());
         vslider->setRenderOffset(getScrollOffset());
      };
      auto setOffsetY = [this](const Slider::SliderValueChangedEvent& event){
         scrollOffsetY.setLerp(event.pct);
         hslider->setRenderOffset(getScrollOffset());
         vslider->setRenderOffset(getScrollOffset());
      };
      subscribe<Slider::SliderValueChangedEvent>(hslider, setOffsetX);
      subscribe<Slider::SliderValueChangedEvent>(vslider, setOffsetY);
   }
   GFCSDraw::ScissorTarget scissorTarget;
   GFCSDraw::Range<double> scrollOffsetX;
   GFCSDraw::Range<double> scrollOffsetY;
   std::shared_ptr<Slider> vslider;
   std::shared_ptr<Slider> hslider;
};

