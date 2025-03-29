#include "Window.h"
#include "ScrollArea.h"
#include "rlgl.h"
using namespace std;
using namespace ReyEngine;
ScrollArea::ScrollArea() {
   _inputFilter = InputFilter::INPUT_FILTER_PROCESS_AND_STOP;
   //create scrollbars
   auto [_vslider, vsnode] = make_node<Slider>(std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
   auto [_hslider, hsnode] = make_node<Slider>(std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
   vslider = _vslider.get();
   hslider = _hslider.get();
   _intrinsicChildren.push_back(std::move(vsnode));
   _intrinsicChildren.push_back(std::move(hsnode));
   vslider->setVisible(true);
   hslider->setVisible(true);

   auto setOffsetX = [this](const Slider::EventSliderValueChanged& event) {
      scrollOffsetX = event.pct;
      updateViewport();
   };
   auto setOffsetY = [this](const Slider::EventSliderValueChanged &event) {
      scrollOffsetY = event.pct;
      updateViewport();
   };
   subscribe<Slider::EventSliderValueChanged>(hslider, setOffsetX);
   subscribe<Slider::EventSliderValueChanged>(vslider, setOffsetY);
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
void ScrollArea::render2DBegin() {
   drawRectangle(getRect(), Colors::blue);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::render2DEnd() {
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
    //get the box that contains all our children, except for the sliders.
    // Make sure it's at least as big as the scrollArea
   boundingBox = getChildBoundingBox();
    auto ourSize = getSize();
    if (boundingBox.width < ourSize.x){
       boundingBox.width = ourSize.x;
    }
    if (boundingBox.height < ourSize.y){
       boundingBox.height = ourSize.y;
    }
    _renderTarget.setSize(boundingBox.size());
    updateViewport();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_rect_changed(Widget* child){
   if (child != vslider && child != hslider) {
      //recalculate
      _on_rect_changed();
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
//void ScrollArea::_on_child_rect_changed(Widget *child) {
//   if (auto isWidget = n->as<Widget>()) {
//      auto& child = isWidget.value();
//      //expand scroll size to fit children
//
//      // reconfigure ourselves when our children's size changes (but ignore the sliders)
//      auto cb = [&](const RectChangedEvent& e) { _on_rect_changed(); };
//
//      if (child != vslider && child != hslider) {
//         //recalculate
//         _on_rect_changed();
//      }
//   }
//}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::updateViewport(){
   auto ourSize = getRect().size();
   _viewport = ourSize.toRect();
   auto vsliderNewRect = ReyEngine::Rect<float>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y) + getPos();
   auto hsliderNewRect = ReyEngine::Rect<float>(0,(ourSize.y - sliderSize), (ourSize.x - sliderSize), sliderSize) + getPos();
   bool xTooBig = boundingBox.width > ourSize.x;
   auto yTooBig = boundingBox.height > ourSize.y;

   //subtract the slider sizes before updating viewport since each dimension depends on the other
   if (xTooBig) _viewport.height = getHeight() - sliderSize; //subtract slider
   if (yTooBig) _viewport.width -= sliderSize; //subtract slider
   _viewport.x = (boundingBox.width - _viewport.width) * (float)Perunum(scrollOffsetX).get();
   _viewport.y = (boundingBox.height - _viewport.height) * (float)Perunum(scrollOffsetY).get();
   if (hslider) {
      hslider->setRect(hsliderNewRect);
      hslider->setVisible(!_hideHSlider && xTooBig);
   }

   if (vslider) {
      vslider->setRect(vsliderNewRect + (vslider->getVisible() ? Size() : Size<float>(sliderSize, 0)));
      vslider->setVisible(!_hideVSlider && yTooBig);
   }
   _window.setSize(_viewport.size());

}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_init(){

}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* ScrollArea::_unhandled_input(const InputEvent& event) {
   //we want to give priority to sliders, then pass input to everyone else
   if (auto isMouse = event.isMouse()) {
      auto& mouse = isMouse.value();
      if (vslider && vslider->_unhandled_input(event)) return vslider;
      if (hslider && hslider->_unhandled_input(event)) return hslider;
      for (auto& child: getChildren()) {
         if (auto isWidget = child->as<Widget>()) {
            auto& widget = isWidget.value();
            if (widget != vslider && widget != hslider) {
               if (widget->_unhandled_input(event)) return widget;
            }
         }
      }
   }
   return nullptr;
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