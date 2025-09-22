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
      _vslider = make_child<Slider>(getNode(), std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
      _hslider = make_child<Slider>(getNode(), std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
      moveToForeground(_vslider.get());
      moveToForeground(_hslider.get());
   }

   _vslider->setVisible(true);
   _hslider->setVisible(true);

   auto setOffsetX = [this](const Slider::EventSliderValueChanged& event) {
      scrollOffsetX = event.pct;
      _on_rect_changed();
   };
   auto setOffsetY = [this](const Slider::EventSliderValueChanged& event) {
      scrollOffsetY = event.pct;
      _on_rect_changed();
   };

   //make sure we only capture outside input when the sliders are being used
   auto cbSliderPress = [this](const Slider::EventSliderPressed& event){_rejectOutsideInput = false;};
   auto cbSliderRelease = [this](const Slider::EventSliderReleased& event){_rejectOutsideInput = true;};

   subscribe<Slider::EventSliderValueChanged>(_hslider, setOffsetX);
   subscribe<Slider::EventSliderValueChanged>(_vslider, setOffsetY);
   subscribe<Slider::EventSliderPressed>(_vslider, cbSliderPress);
   subscribe<Slider::EventSliderPressed>(_hslider, cbSliderPress);
   subscribe<Slider::EventSliderReleased>(_vslider, cbSliderRelease);
   subscribe<Slider::EventSliderReleased>(_hslider, cbSliderRelease);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideVSlider(bool hidden) {
    _hideVSlider = hidden;
    if (_vslider) _vslider->setVisible(!hidden);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideHSlider(bool hidden) {
    _hideHSlider = hidden;
    if (_hslider) _hslider->setVisible(!hidden);
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
   if (_hslider) {
      _hslider->setRect(hsliderNewRect);
      _hslider->setVisible(!_hideHSlider && needShowHSlider);
   }

   if (_vslider) {
      _vslider->setRect(vsliderNewRect);
      _vslider->setVisible(!_hideVSlider && needShowVSlider);
   }
   camera.offset = {-_viewport.pos().x, -_viewport.pos().y};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_rect_changed(Widget* child){
   if (child != _vslider.get() && child != _hslider.get()) {
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
Widget* ScrollArea::_unhandled_input(const InputEvent& e) {
   switch (e.eventId){
      case InputEventMouseWheel::ID:{
         constexpr float SPEED_FACTOR = 2.0;
         auto& mwEvent = e.toEvent<InputEventMouseWheel>();
         bool handle = false;
         if (_hslider && _hslider->_visible){
            _hslider->setSliderValue(_hslider->getSliderValue() - mwEvent.wheelMove.x * SPEED_FACTOR);
            handle = true;
         }
         if (_vslider && _vslider->_visible){
            _vslider->setSliderValue(_vslider->getSliderValue() - mwEvent.wheelMove.y * SPEED_FACTOR);
            handle = true;
         }
         if (handle) return this;
      }
   }
   return nullptr;
}