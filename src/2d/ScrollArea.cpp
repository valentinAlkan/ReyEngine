#include "Window.h"
#include "ScrollArea.h"
#include "rlgl.h"
#include "Label.h"
using namespace std;
using namespace ReyEngine;

ScrollArea::ScrollArea() {
   _rejectOutsideInput = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_init() {
   //create scrollbars
   {
      auto [_vslider, vsnode] = make_node<Slider>(std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
      auto [_hslider, hsnode] = make_node<Slider>(std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
      addChild(std::move(vsnode));
      addChild(std::move(hsnode));
      moveToForeground(_vslider.get());
      moveToForeground(_hslider.get());
      vslider = _vslider.get();
      hslider = _hslider.get();
   }

   vslider->setVisible(true);
   hslider->setVisible(true);

   auto setOffsetX = [this](const Slider::EventSliderValueChanged& event) {
      scrollOffsetX = event.pct;
      _on_rect_changed();
   };
   auto setOffsetY = [this](const Slider::EventSliderValueChanged& event) {
      scrollOffsetY = event.pct;
      _on_rect_changed();
   };

   //make sure we capture outside input only when the sliders are being used
   auto cbSliderPress = [this](const Slider::EventSliderPressed& event){_rejectOutsideInput = false;};
   auto cbSliderRelease = [this](const Slider::EventSliderReleased& event){_rejectOutsideInput = true;};

   subscribe<Slider::EventSliderValueChanged>(hslider, setOffsetX);
   subscribe<Slider::EventSliderValueChanged>(vslider, setOffsetY);
   subscribe<Slider::EventSliderPressed>(vslider, cbSliderPress);
   subscribe<Slider::EventSliderPressed>(hslider, cbSliderPress);
   subscribe<Slider::EventSliderReleased>(vslider, cbSliderRelease);
   subscribe<Slider::EventSliderReleased>(hslider, cbSliderRelease);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideVSlider(bool hidden) {
    _hideVSlider = hidden;
    if (vslider) vslider->setVisible(!hidden);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideHSlider(bool hidden) {
    _hideHSlider = hidden;
    if (hslider) hslider->setVisible(!hidden);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
    //get the box that contains all our children, except for the sliders.
    // Make sure it's at least as big as the scrollArea
   _viewport = getSizeRect();
    boundingBox = getChildBoundingBox();
   auto ourSize = getSize();
   if (boundingBox.width < ourSize.x){
      boundingBox.width = ourSize.x;
   }
   if (boundingBox.height < ourSize.y){
      boundingBox.height = ourSize.y;
   }
   if (boundingBox.size() != Size<R_FLOAT>(_renderTarget.getSize()) && _autoResize) {
      _renderTarget.setSize(boundingBox.size());
   }

   auto areaSize = getRect().size();
   auto vsliderNewRect = Rect<float>((areaSize.x - sliderSize), 0, sliderSize, areaSize.y) + getPos();
   auto hsliderNewRect = Rect<float>(0, (areaSize.y - sliderSize), (areaSize.x - sliderSize), sliderSize) + getPos();
   bool needShowHSlider = boundingBox.width > _viewport.width;
   auto needShowVSlider = boundingBox.height > _viewport.height;
   if (!needShowVSlider) hsliderNewRect.stretchRight(sliderSize);

   //subtract the slider sizes before updating viewport since each dimension depends on the other
   if (needShowHSlider) _viewport.height -= sliderSize;
   if (needShowVSlider) _viewport.width -= sliderSize;
   _viewport.x = (boundingBox.width - areaSize.x) * (float)Fraction(scrollOffsetX).get();
   _viewport.y = (boundingBox.height - areaSize.y) * (float)Fraction(scrollOffsetY).get();
   if (hslider) {
      hslider->setRect(hsliderNewRect);
      hslider->setVisible(!_hideHSlider && needShowHSlider);
   }

   if (vslider) {
      vslider->setRect(vsliderNewRect);
      vslider->setVisible(!_hideVSlider && needShowVSlider);
   }
   camera.offset = {-_viewport.pos().x, -_viewport.pos().y};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_rect_changed(Widget* child){
   if (child != vslider && child != hslider) {
      //recalculate
      _on_rect_changed();
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_added_to_tree(TypeNode* typeNode) {
   if (auto isWidget = typeNode->as<Widget>()) {
      _on_child_rect_changed(isWidget.value());
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
//std::optional<Widget*> ScrollArea::askHover(const ReyEngine::Pos<R_FLOAT>& globalPos) {
//    //pass to childwidgets
//    if (vslider && vslider->askHover(globalPos)) return vslider;
//    if (hslider && hslider->askHover(globalPos)) return hslider;
//    for (auto &child: getChildren()) {
//        if (child != vslider && child != hslider) {
//            auto handled = child->askHover(globalPos);
//            if (handled) return handled;
//        }
//    }
//    return nullopt;
//}