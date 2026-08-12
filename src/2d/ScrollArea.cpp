#include "Window.h"
#include "ScrollArea.h"
#include "rlgl.h"
#include "Label.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Handled Internal::ScrollAreaViewport::_processInput(const InputEvent& e) {
   //input has to stop at the same edge the scissor does, otherwise clipped-away content
   //is still clickable.
   if (auto isMouse = e.isMouse(); isMouse && !isMouse.value()->isInside()) return nullptr;
   return Widget::_processInput(e);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::ScrollAreaContent::_on_child_rect_changed(Widget*) {
   _owner.refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::ScrollAreaContent::_on_child_added_to_tree(TypeNode*) {
   _owner.refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::render2D(RenderContext&) const {
   //the old Canvas-based implementation got this from ClearBackground on its render target
   drawRectangle(getSizeRect(), getTheme().background.colorPrimary);
}

/////////////////////////////////////////////////////////////////////////////////////////
ScrollArea::ScrollArea() {
   _ignoreOutsideInput = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
TypeNode* ScrollArea::getChildInsertionNode() {
   //before _init runs we have no content node yet, so internal setup lands on us directly
   return _content ? _content->getNode() : getNode();
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<R_FLOAT> ScrollArea::getViewportSize() const {
   return {_scroll.axisX().viewport(), _scroll.axisY().viewport()};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_init() {
   //Build our internal children against getNode() explicitly - passing the raw TypeNode
   //bypasses getChildInsertionNode(), which is what keeps them out of the content area.
   _viewport = make_child<Internal::ScrollAreaViewport>(getNode(), std::string(VIEWPORT_NAME));
   _content = make_child<Internal::ScrollAreaContent>(_viewport->getNode(), std::string(CONTENT_NAME), *this);
   _vslider = make_child<Slider>(getNode(), std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
   _hslider = make_child<Slider>(getNode(), std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);

   _scroll.attachVBar(_vslider);
   _scroll.attachHBar(_hslider);

   //bar drag -> offset. fromBar so the model doesn't write back and fight the drag.
   subscribe<Slider::EventSliderValueChanged>(_hslider, [this](const auto& e){
      _scroll.setOffsetX((float)e.value, /*fromBar*/true);
      applyOffset();
   });
   subscribe<Slider::EventSliderValueChanged>(_vslider, [this](const auto& e){
      _scroll.setOffsetY((float)e.value, /*fromBar*/true);
      applyOffset();
   });

   //Focus the slider for the duration of a drag so it keeps receiving input once the
   //mouse leaves our rect. Focus lives on the owning canvas, which is now somewhere
   //above us rather than being us.
   auto cbSliderPress = [](Slider::EventSliderPressed& event){
      event.publisher->template asMut<Slider>().value()->setFocused(true);
   };
   auto cbSliderRelease = [](Slider::EventSliderReleased& event){
      event.publisher->template asMut<Slider>().value()->setFocused(false);
   };
   subscribeMutable<Slider::EventSliderPressed>(_vslider, cbSliderPress);
   subscribeMutable<Slider::EventSliderPressed>(_hslider, cbSliderPress);
   subscribeMutable<Slider::EventSliderReleased>(_vslider, cbSliderRelease);
   subscribeMutable<Slider::EventSliderReleased>(_hslider, cbSliderRelease);

   refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideVSlider(bool hidden) {
   _hideVSlider = hidden;
   refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::hideHSlider(bool hidden) {
   _hideHSlider = hidden;
   refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::setOffsetX(const Percent& pct) {
   _scroll.setOffsetX(_scroll.maxOffsetX() * Fraction(pct).get());
   applyOffset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::setOffsetY(const Percent& pct) {
   _scroll.setOffsetY(_scroll.maxOffsetY() * Fraction(pct).get());
   applyOffset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::setOffsetX(float amt) {
   _scroll.setOffsetX(amt); //clamped by the model
   applyOffset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::setOffsetY(float amt) {
   _scroll.setOffsetY(amt); //clamped by the model
   applyOffset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::applyOffset() {
   if (!_content) return;
   //scrolling forward moves the content backward
   const auto offset = _scroll.offset();
   const Pos<R_FLOAT> newPos = {-offset.x, -offset.y};
   if (_content->getPos() == newPos) return;
   //guard: moving the content notifies us right back as a child-rect change
   const bool wasRefreshing = _refreshing;
   _refreshing = true;
   _content->setPosition(newPos);
   _refreshing = wasRefreshing;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::refreshScroll() {
   if (_refreshing) return;   //re-entered from our own child updates
   if (!_content) return;     //_init hasn't run yet
   _refreshing = true;

   const auto ourSize = getSize();

   //How far the content reaches. getChildBoundingBox() already unions the content node's
   //own size rect with its children, so this is never smaller than the viewport.
   const auto contentBox = _content->getChildBoundingBox();
   const Size<R_FLOAT> contentSize = {std::max(contentBox.width, ourSize.x),
                                      std::max(contentBox.height, ourSize.y)};

   //Resolves both axes together: a bar on one axis eats into the other's viewport, which
   //can be exactly what makes the other bar necessary.
   _scroll.layout(contentSize, ourSize, sliderSize);

   //the model sets bar visibility from need; the hide flags override it
   if (_vslider) _vslider->setVisible(!_hideVSlider && _scroll.needsVBar());
   if (_hslider) _hslider->setVisible(!_hideHSlider && _scroll.needsHBar());

   const auto viewportSize = getViewportSize();
   if (_viewport) _viewport->setRect({{0, 0}, viewportSize});
   //content matches the viewport so anchored/FILL children fill the visible area
   if (_content->getSize() != viewportSize) _content->setSize(viewportSize);

   if (_vslider) _vslider->setRect({ourSize.x - sliderSize, 0, sliderSize, ourSize.y});
   if (_hslider) _hslider->setRect({0, ourSize.y - sliderSize, ourSize.x - sliderSize, sliderSize});

   _refreshing = false;
   applyOffset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
   refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_rect_changed(Widget* child){
   //our own children are the viewport and the sliders, all of which we place ourselves
   if (child == _viewport.get() || child == _vslider.get() || child == _hslider.get()) return;
   refreshScroll();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ScrollArea::_processInput(const InputEvent& e) {
   //Canvas used to do this for us. Focused/modal widgets still need the event even when
   //the pointer has left us - that's what keeps a slider drag alive.
   if (_ignoreOutsideInput && !isFocused() && !isModal()) {
      if (auto isMouse = e.isMouse(); isMouse && !isMouse.value()->isInside()) return nullptr;
   }
   return Widget::_processInput(e);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ScrollArea::_unhandled_input(const InputEvent& e) {
   switch (e.eventId){
      case InputEventMouseWheel::ID:{
         constexpr float WHEEL_SPEED = 30.0f; //pixels per wheel notch, matching Tree
         auto& mwEvent = e.toEvent<InputEventMouseWheel>();
         bool handled = false;
         if (_scroll.needsHBar() && mwEvent.wheelMove.x != 0){
            _scroll.scrollByX(-mwEvent.wheelMove.x * WHEEL_SPEED);
            handled = true;
         }
         if (_scroll.needsVBar() && mwEvent.wheelMove.y != 0){
            _scroll.scrollByY(-mwEvent.wheelMove.y * WHEEL_SPEED);
            handled = true;
         }
         if (handled) {
            applyOffset();
            return {this, e.isMouse().value()->getLocalPos()};
         }
      }
   }
   return nullptr;
}
