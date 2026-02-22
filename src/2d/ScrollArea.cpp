#include "Window.h"
#include "ScrollArea.h"
#include "rlgl.h"
#include "Label.h"
using namespace std;
using namespace ReyEngine;

void ScrollArea::render2D() const {
}
/////////////////////////////////////////////////////////////////////////////////////////
ScrollArea::ScrollArea() {
   _ignoreOutsideInput = true;
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
   auto cbSliderPress = [this](const Slider::EventSliderPressed& event){ _ignoreOutsideInput = false;};
   auto cbSliderRelease = [this](const Slider::EventSliderReleased& event){ _ignoreOutsideInput = true;};

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

   auto getBackgroundChildBoundingBox = [&](){
      Rect<R_FLOAT> retval = getSizeRect();
      for (const auto& child : _background.getValues()){
         if (auto isWidget = child->as<Widget>()) {
            auto& _child = isWidget.value();
            retval = retval.combine(_child->getRect());
         }
      }
      return retval;
   };

   boundingBox = getBackgroundChildBoundingBox();
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

   const auto areaSize = getRect().size();
   const auto vsliderNewRect = Rect<float>((areaSize.x - sliderSize), 0, sliderSize, areaSize.y) + getPos();
   const auto hsliderNewRect = Rect<float>(0, (areaSize.y - sliderSize), (areaSize.x - sliderSize), sliderSize) + getPos();
   const bool needShowHSlider = boundingBox.width > _viewport.width;
   const auto needShowVSlider = boundingBox.height > _viewport.height;

   //subtract the slider sizes before updating viewport since each dimension depends on the other
   if (needShowHSlider) _viewport.height -= sliderSize;
   if (needShowVSlider) _viewport.width -= sliderSize;
   _viewport.x = (boundingBox.width - areaSize.x) * (float)Fraction(scrollOffsetX).get();
   _viewport.y = (boundingBox.height - areaSize.y) * (float)Fraction(scrollOffsetY).get();
   if (_hslider) {
      _hslider->setRect(hsliderNewRect);
      _hslider->setVisible(!_hideHSlider && needShowHSlider);
      // Set visible amount so grabber size reflects viewport/content ratio
      auto scrollableWidth = boundingBox.width - _viewport.width;
      if (scrollableWidth > 0) {
         _hslider->setVisibleAmount(100.0 * _viewport.width / scrollableWidth);
      }
   }

   if (_vslider) {
      _vslider->setRect(vsliderNewRect);
      _vslider->setVisible(!_hideVSlider && needShowVSlider);
      // Set visible amount so grabber size reflects viewport/content ratio
      auto scrollableHeight = boundingBox.height - _viewport.height;
      if (scrollableHeight > 0) {
         _vslider->setVisibleAmount(100.0 * _viewport.height / scrollableHeight);
      }
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
      case InputEventMouseMotion::ID:{
         auto& mmEvent = e.toEvent<InputEventMouseMotion>();
      } break;
      case InputEventMouseWheel::ID:{
         constexpr float SPEED_FACTOR = 5.0;
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