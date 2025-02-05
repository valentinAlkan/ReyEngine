#include "ScrollArea.h"

using namespace std;
using namespace ReyEngine;

constexpr bool scrollAreaDebug = true;

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
   //similar to a canvas
   Application::getWindow(0).pushRenderTarget(_renderTarget);
   _renderTarget.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::renderEnd() {
   drawRectangleLines(_childBoundingBox.toRect(), 1, Colors::red);
   Application::getWindow(0).popRenderTarget();
   if constexpr (scrollAreaDebug){
      //draw everything, and show a debug view of the sliding window
      auto r = _renderTarget.getSize().toRect();
      drawRenderTargetRect(_renderTarget, r, r, theme->background.colorPrimary.value);
      drawRectangleLines(getChildBoundingBox(), 2.0, Colors::green);
   } else {
      drawRenderTargetRect(_renderTarget, _viewport, _window, theme->background.colorPrimary.value);
   }
   //redraw the sliders so that they're always on top of the children
   hslider->renderChain();
   vslider->renderChain();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
    //reset to 0 so we don't get weird bugs
    scrollOffsetX.setValue(0);
    scrollOffsetY.setValue(0);
    auto ourSize= getRect().size();
    static constexpr float sliderSize = 20;
    //get the box that contains all our children, except for the sliders
    _childBoundingBox = getScrollAreaChildBoundingBox().max(ourSize);
    _renderTarget.setSize(_childBoundingBox);
    scrollOffsetX.setMax(_childBoundingBox.x - getWidth());
    scrollOffsetY.setMax(_childBoundingBox.y - getHeight());
    auto vsliderNewRect = ReyEngine::Rect<float>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y) + getPos();
    auto hsliderNewRect = ReyEngine::Rect<float>(0,(ourSize.y - sliderSize), (ourSize.x - sliderSize), sliderSize) + getPos();
    if (vslider) {
        vslider->setRect(vsliderNewRect);
        vslider->setVisible(!_hideVSlider && _childBoundingBox.y > getHeight());
        _viewport.width = getWidth() - vsliderNewRect.width;
    }
    if (hslider) {
        //add in the width of the vslider if it is not visible
        hslider->setRect(hsliderNewRect + (vslider->getVisible() ? Size() : Size<float>(sliderSize, 0)));
        hslider->setVisible(!_hideHSlider && _childBoundingBox.x > getWidth());
        _viewport.height = getHeight() - hsliderNewRect.height;
    }
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
   _viewport = {scrollOffsetX.getValue(), scrollOffsetY.getValue(), getWidth(), getHeight()};
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
        scrollOffsetX.setLerp(event.pct);
        updateViewport();
    };
    auto setOffsetY = [this](const Slider::EventSliderValueChanged &event) {
        scrollOffsetY.setLerp(event.pct);
       updateViewport();
    };
    subscribe<Slider::EventSliderValueChanged>(hslider, setOffsetX);
    subscribe<Slider::EventSliderValueChanged>(vslider, setOffsetY);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<R_FLOAT> ScrollArea::getScrollAreaChildBoundingBox() {
   //just like normal child bounding box but ignores scroll bars
   Size<R_FLOAT> childRect;
   for (const auto &child: getChildren()) {
      if (child->getName() == VSLIDER_NAME) {
//         if (vslider->getVisible()) childRect.x += vslider->getWidth();
         continue;
      } else if (child->getName() == HSLIDER_NAME) {
//         if (hslider->getVisible()) childRect.y += hslider->getHeight();
         continue;
      }
      childRect = childRect.max(child->getRect().enclosing().size());
   }
   return childRect;
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
                if (child->_process_unhandled_input(event, child->toMouseInput(globalPos - getScrollOffset()))) return true;
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