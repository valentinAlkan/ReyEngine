#pragma once
#include "Control.hpp"
#include "Slider.hpp"

class ScrollArea : public Control {
   GFCSDRAW_OBJECT(ScrollArea, Control)
   , scrollOffsetX(0, 0, 0)
   , scrollOffsetY(0, 0, 0)
   {
      _setIsEditorWidget(true);
      _editor_show_grab_handles = true;
   }
public:
   inline GFCSDraw::Size<double> getScrollAreaSize() const {return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};}
   inline GFCSDraw::Pos<double> getScrollOffset() const {return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};}
   void hideVSlider(bool hidden){_hideVSlider = hidden; if (vslider) vslider->setVisible(!hidden);}
   void hideHSlider(bool hidden){_hideHSlider = hidden; if (hslider) hslider->setVisible(!hidden);}
protected:
   static constexpr std::string_view VSLIDER_NAME = "__vslider";
   static constexpr std::string_view HSLIDER_NAME = "__hslider";
   void renderBegin(GFCSDraw::Pos<double>& textureOffset) override {
//      textureOffset -= getScrollOffset();
//      scissorTarget.start(getGlobalRect());
   }
   void renderEnd() override {
//      scissorTarget.stop();
//      int size = 20;
//      _drawRectangle({0,0,size,size}, getThemeReadOnly().background.colorPrimary.value);
//      _drawRectangle({0,getHeight()-size,size,size}, getThemeReadOnly().background.colorPrimary.value);
//      _drawRectangle({getWidth()-size,0,size,size}, getThemeReadOnly().background.colorPrimary.value);
//      _drawRectangle({getWidth()-size,getHeight()-size,size,size}, getThemeReadOnly().background.colorPrimary.value);
   }
   void _process(float dt) override {}
   void registerProperties() override{
   }
   void _on_rect_changed() override {
      auto ourSize = _rect.get().size();
      auto sliderSize = 20;
      auto newSize = getRect().size();
      scrollOffsetX.setRange(0, newSize.x);
      scrollOffsetY.setRange(0, newSize.y);
      auto vsliderNewRect = GFCSDraw::Rect<int>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y);
      auto hsliderNewRect = GFCSDraw::Rect<int>(0, (float)(ourSize.y - sliderSize), (float)(ourSize.x - sliderSize), (float)sliderSize);
      vslider->setRect(vsliderNewRect);
      hslider->setRect(hsliderNewRect);
   }
   void _on_child_added(std::shared_ptr<BaseWidget>& child) override {
      //expand scroll size to fit children
      auto rectSize = getChildRectSize();
      if (!isInLayout) {
         scrollOffsetX.setMax(rectSize.x);
         scrollOffsetY.setMax(rectSize.y);
      }
      // reconfigure ourselves when our children's size changes (but ignore the sliders)
      auto cb = [&](const WidgetResizeEvent& e){_on_rect_changed();};

      if (child != vslider && child != hslider) {
         subscribe<WidgetResizeEvent>(child, cb);
      }
   }

protected:
   void _init() override {
      //create scrollbars
      vslider = std::make_shared<Slider>(std::string(VSLIDER_NAME), GFCSDraw::Rect<float>(), Slider::SliderType::VERTICAL);
      hslider = std::make_shared<Slider>(std::string(HSLIDER_NAME), GFCSDraw::Rect<float>(), Slider::SliderType::HORIZONTAL);
      vslider->setVisible(!_hideVSlider);
      hslider->setVisible(!_hideHSlider);
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
   bool _hideVSlider = true; //always hidden until we get a child
   bool _hideHSlider = true; //always hidden until we get a child
};

