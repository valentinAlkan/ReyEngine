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
      textureOffset -= getScrollOffset();
      scissorTarget.start(getGlobalRect());
   }
   void renderEnd() override {
      scissorTarget.stop();
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
      if (vslider){
         vslider->setRect(vsliderNewRect);
         hslider->setVisible(!_hideHSlider && _childBoundingBox.x > getWidth());
      }
      if (hslider) {
         hslider->setRect(hsliderNewRect);
         vslider->setVisible(!_hideVSlider && _childBoundingBox.y > getHeight());
      }
   }
   void _on_child_added(std::shared_ptr<BaseWidget>& child) override {
      //expand scroll size to fit children
      _childBoundingBox = getChildBoundingBox();
      if (!isInLayout) {
         scrollOffsetX.setMax(_childBoundingBox.x);
         scrollOffsetY.setMax(_childBoundingBox.y);
      }
      // reconfigure ourselves when our children's size changes (but ignore the sliders)
      auto cb = [&](const WidgetResizeEvent& e){_on_rect_changed();};

      if (child != vslider && child != hslider) {
         subscribe<WidgetResizeEvent>(child, cb);
      }
      //recalculate
      _on_rect_changed();
   }

protected:
   void _init() override {
      //create scrollbars
      vslider = std::make_shared<Slider>(std::string(VSLIDER_NAME), GFCSDraw::Rect<float>(), Slider::SliderType::VERTICAL);
      hslider = std::make_shared<Slider>(std::string(HSLIDER_NAME), GFCSDraw::Rect<float>(), Slider::SliderType::HORIZONTAL);
      vslider->setVisible(false);
      hslider->setVisible(false);
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
   GFCSDraw::Size<int> _childBoundingBox;
   bool _hideVSlider = false;
   bool _hideHSlider = false;
};

