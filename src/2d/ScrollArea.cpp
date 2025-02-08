#include "ScrollArea.h"
#include "rlgl.h"
using namespace std;
using namespace ReyEngine;
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
void ScrollArea::renderBegin() {
   //draw background
   drawRectangle(getRect(), theme->background.colorPrimary);
   //similar to a canvas
   Application::getWindow(0).pushRenderTarget(_renderTarget);

   //undo our specific offsets, which are going to be re-applied by the base renderchain
   // this WILL affect any rendering we try to do in the local space
   rlPushMatrix();
   rlScalef(1 / transform.scale.x, 1 / transform.scale.y, 1);
   rlRotatef(-transform.rotation * 180 / M_PI, 0, 0, 1);
   rlTranslatef(-transform.position.x, -transform.position.y, 0);
   _renderTarget.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::render() const {
 /* any drawing done here will be modified by the inverse of the translation matrix*/
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::renderEnd() {
   //done drawing children, we need to reload the old transform so we can draw the rendertarget where its supposed to be
   rlPopMatrix();
   Application::getWindow(0).popRenderTarget();
   if constexpr (false){
      //debug
      //draw everything, and show a debug view of the sliding window
      constexpr Pos<float> DEBUG_OFFSET = {600, 0};
      drawRenderTargetRect(_renderTarget, _renderBox, _renderBox + DEBUG_OFFSET, theme->background.colorPrimary.value);
      drawRectangleLines(_renderBox + getPos() + DEBUG_OFFSET, 2.0, Colors::green);
      drawRectangleLines(_viewport + getPos() + DEBUG_OFFSET, 1.0, Colors::blue);
      drawRectangleLines(_window + getPos() + DEBUG_OFFSET, 1.0, Colors::yellow);
      drawText("viewport", _viewport.pos() + getPos() + DEBUG_OFFSET);
      drawText("viewport = " + _viewport.toString(), getPos() + DEBUG_OFFSET - Pos<float>(0, 40));
      drawText("window = " + _window.toString(), getPos() + DEBUG_OFFSET - Pos<float>(0, 20));
   } else {
      //normal
      drawRenderTargetRect(_renderTarget, _viewport, _window, theme->background.colorPrimary.value);
   }
   //redraw the sliders so that they're always on top of the children
   hslider->renderChain();
   vslider->renderChain();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
    //get the box that contains all our children, except for the sliders.
    // Make sure it's at least as big as the scrollArea
   _renderBox = getChildBoundingBox();
    auto ourSize = getSize();
    if (_renderBox.width < ourSize.x){
       _renderBox.width = ourSize.x;
    }
    if (_renderBox.height < ourSize.y){
       _renderBox.height = ourSize.y;
    }
    _renderTarget.setSize(_renderBox.size());
    updateViewport();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_rect_changed(std::shared_ptr<BaseWidget>& child){
   if (child != vslider && child != hslider) {
      //recalculate
      _on_rect_changed();
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_added(std::shared_ptr<BaseWidget>& child){
    //expand scroll size to fit children

    // reconfigure ourselves when our children's size changes (but ignore the sliders)
    auto cb = [&](const WidgetRectChangedEvent &e) { _on_rect_changed(); };

    if (child != vslider && child != hslider) {
       //recalculate
       _on_rect_changed();
       subscribe<WidgetRectChangedEvent>(child, cb);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::updateViewport(){
   auto ourSize = getRect().size();
   _viewport = ourSize.toRect();
   auto vsliderNewRect = ReyEngine::Rect<float>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y) + getPos();
   auto hsliderNewRect = ReyEngine::Rect<float>(0,(ourSize.y - sliderSize), (ourSize.x - sliderSize), sliderSize) + getPos();
   bool xTooBig = _renderBox.width > ourSize.x;
   auto yTooBig = _renderBox.height > ourSize.y;

   //subtract the slider sizes before updating viewport since each dimension depends on the other
   if (xTooBig) _viewport.height = getHeight() - sliderSize; //subtract slider
   if (yTooBig) _viewport.width -= sliderSize; //subtract slider
   _viewport.x = (_renderBox.width - _viewport.width) * Perunum(scrollOffsetX).get();
   _viewport.y = (_renderBox.height - _viewport.height) * Perunum(scrollOffsetY).get();
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
    //cancel out input offset from base canvas
    setInputOffset({0, 0});
    //create scrollbars
    vslider = Slider::build(std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
    hslider = Slider::build(std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
    vslider->setVisible(false);
    hslider->setVisible(false);


    auto setOffsetX = [this](const Slider::EventSliderValueChanged &event) {
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
Handled ScrollArea::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
    //we want to give priority to sliders, then pass input to everyone else
    if (mouse) {
        auto globalPos = localToGlobal(mouse->localPos);
        if (vslider && vslider->_process_unhandled_input(event, vslider->toMouseInput(globalPos))) return true;
        if (hslider && hslider->_process_unhandled_input(event, hslider->toMouseInput(globalPos))) return true;
        for (auto &child: getChildren()) {
            if (child != vslider && child != hslider) {
                if (child->_process_unhandled_input(event, child->toMouseInput(globalPos))) return true;
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> ScrollArea::askHover(const ReyEngine::Pos<R_FLOAT>& globalPos) {
    //pass to childwidgets
    if (vslider && vslider->askHover(globalPos)) return vslider;
    if (hslider && hslider->askHover(globalPos)) return hslider;
    for (auto &child: getChildren()) {
        if (child != vslider && child != hslider) {
            auto handled = child->askHover(globalPos);
            if (handled) return handled;
        }
    }
    return nullopt;
}