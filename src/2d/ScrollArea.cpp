#include "ScrollArea.h"

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
void ScrollArea::renderBegin(ReyEngine::Pos<double>& textureOffset){
    textureOffset -= getScrollOffset();
    startScissor(_rect.value.toSizeRect());
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::renderEnd() {
    stopScissor();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_rect_changed(){
    //reset to 0 so we don't get weird bugs
    scrollOffsetX.setValue(0);
    scrollOffsetY.setValue(0);
    auto ourSize = _rect.value.size();
    static constexpr int sliderSize = 20;
    //get the box that contains all our children, except for the sliders
    _childBoundingBox = getScrollAreaChildBoundingBox();
    scrollOffsetX.setMax(_childBoundingBox.x - getWidth());
    scrollOffsetY.setMax(_childBoundingBox.y - getHeight());
    _inputMask.mask = ReyEngine::Rect<int>(0, 0, (ourSize.x - sliderSize), ourSize.y - sliderSize);
    auto vsliderNewRect = ReyEngine::Rect<int>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y);
    auto hsliderNewRect = ReyEngine::Rect<int>(0,(ourSize.y - sliderSize), (ourSize.x - sliderSize), sliderSize);
    if (vslider) {
        vslider->setRect(vsliderNewRect);
        vslider->setVisible(!_hideVSlider && _childBoundingBox.y > _rect.value.height);
    }
    if (hslider) {
        //add in the width of the vslider if it is not visible
        hslider->setRect(hsliderNewRect + (vslider->getVisible() ? Size<int>() : Size<int>{sliderSize, 0}));
        hslider->setVisible(!_hideHSlider && _childBoundingBox.x > _rect.value.width);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_on_child_added(std::shared_ptr<BaseWidget>& child){
    //expand scroll size to fit children

    // reconfigure ourselves when our children's size changes (but ignore the sliders)
    auto cb = [&](const WidgetResizeEvent &e) { _on_rect_changed(); };

    if (child != vslider && child != hslider) {
        subscribe<WidgetResizeEvent>(child, cb);
    }
    //recalculate
    _on_rect_changed();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ScrollArea::_init(){
    //turn on input masking
    _inputMask.value = InputMask::IGNORE_OUTSIDE;
    //create scrollbars
    vslider = std::make_shared<Slider>(std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
    hslider = std::make_shared<Slider>(std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
    vslider->setVisible(false);
    hslider->setVisible(false);
    addChild(vslider);
    addChild(hslider);

    auto setOffsetX = [this](const Slider::EventSliderValueChanged &event) {
        scrollOffsetX.setLerp(event.pct);
        hslider->setRenderOffset(getScrollOffset());
        vslider->setRenderOffset(getScrollOffset());
    };
    auto setOffsetY = [this](const Slider::EventSliderValueChanged &event) {
        scrollOffsetY.setLerp(event.pct);
        hslider->setRenderOffset(getScrollOffset());
        vslider->setRenderOffset(getScrollOffset());
    };
    subscribe<Slider::EventSliderValueChanged>(hslider, setOffsetX);
    subscribe<Slider::EventSliderValueChanged>(vslider, setOffsetY);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> ScrollArea::getScrollAreaChildBoundingBox() {
    ReyEngine::Size<double> childRect;
    for (const auto &child: getChildren()) {
        if (child->getName() == VSLIDER_NAME) {
            if (vslider->getVisible()) childRect.x += vslider->getWidth();
            continue;
        } else if (child->getName() == HSLIDER_NAME) {
            if (hslider->getVisible()) childRect.y += hslider->getHeight();
            continue;
        }
        auto totalOffset = child->getRect().size() + ReyEngine::Size<double>(child->getPos());
        childRect.max(totalOffset);
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
                if (child->_process_unhandled_input(event, child->toMouseInput(globalPos))) return true;
            }
        }
    }
    return false;
}
