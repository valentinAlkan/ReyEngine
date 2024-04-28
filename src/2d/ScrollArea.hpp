#pragma once
#include "Control.h"
#include "Slider.hpp"

namespace ReyEngine {
    class ScrollArea : public BaseWidget {
    REYENGINE_OBJECT(ScrollArea, BaseWidget), scrollOffsetX(0, 0, 0), scrollOffsetY(0, 0, 0) {}

    public:
        inline ReyEngine::Size<double> getScrollAreaSize() const {
            return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};
        }

        inline ReyEngine::Pos<double> getScrollOffset() const {
            return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};
        }

        void hideVSlider(bool hidden) {
            _hideVSlider = hidden;
            if (vslider) vslider->setVisible(!hidden);
        }

        void hideHSlider(bool hidden) {
            _hideHSlider = hidden;
            if (hslider) hslider->setVisible(!hidden);
        }

    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void renderBegin(ReyEngine::Pos<double> &textureOffset) override {
            textureOffset -= getScrollOffset();
//      scissorTarget.begin();
        }

        void render() const {}

        void renderEnd() override {
//      scissorTarget.end();
        }

        void _process(float dt) override {}

        void registerProperties() override {
        }

        void _on_rect_changed() override {
            //reset to 0 so we don't get weird bugs
            scrollOffsetX.setValue(0);
            scrollOffsetY.setValue(0);
            auto ourSize = _rect.value.size();
            auto sliderSize = 20;
            _childBoundingBox = getScrollAreaChildBoundingBox();
            scrollOffsetX.setMax(_childBoundingBox.x - getWidth());
            scrollOffsetY.setMax(_childBoundingBox.y - getHeight());
            auto vsliderNewRect = ReyEngine::Rect<int>((ourSize.x - sliderSize), 0, sliderSize, ourSize.y);
            auto hsliderNewRect = ReyEngine::Rect<int>(0, (float) (ourSize.y - sliderSize),
                                                       (float) (ourSize.x - sliderSize), (float) sliderSize);
            if (vslider) {
                vslider->setRect(vsliderNewRect);
                hslider->setVisible(!_hideHSlider && _childBoundingBox.x > getWidth());
            }
            if (hslider) {
                hslider->setRect(hsliderNewRect);
                vslider->setVisible(!_hideVSlider && _childBoundingBox.y > getHeight());
            }
            scissorTarget = getRect();
        }

        void _on_child_added(std::shared_ptr<BaseWidget> &child) override {
            //expand scroll size to fit children

            // reconfigure ourselves when our children's size changes (but ignore the sliders)
            auto cb = [&](const WidgetResizeEvent &e) { _on_rect_changed(); };

            if (child != vslider && child != hslider) {
                subscribe<WidgetResizeEvent>(child, cb);
            }
            //recalculate
            _on_rect_changed();
        }

    protected:
        void _init() override {
            //create scrollbars
            vslider = std::make_shared<Slider>(std::string(VSLIDER_NAME), Slider::SliderType::VERTICAL);
            hslider = std::make_shared<Slider>(std::string(HSLIDER_NAME), Slider::SliderType::HORIZONTAL);
            vslider->setVisible(false);
            hslider->setVisible(false);
            addChild(vslider);
            addChild(hslider);

            auto setOffsetX = [this](const Slider::SliderValueChangedEvent &event) {
                scrollOffsetX.setLerp(event.pct);
                hslider->setRenderOffset(getScrollOffset());
                vslider->setRenderOffset(getScrollOffset());
            };
            auto setOffsetY = [this](const Slider::SliderValueChangedEvent &event) {
                scrollOffsetY.setLerp(event.pct);
                hslider->setRenderOffset(getScrollOffset());
                vslider->setRenderOffset(getScrollOffset());
            };
            subscribe<Slider::SliderValueChangedEvent>(hslider, setOffsetX);
            subscribe<Slider::SliderValueChangedEvent>(vslider, setOffsetY);
        }

        ///Ignores scroll bars, but adds their widths if they are visible
        ReyEngine::Size<int> getScrollAreaChildBoundingBox() {
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


        ReyEngine::Rect<int> scissorTarget;
        ReyEngine::Range<double> scrollOffsetX;
        ReyEngine::Range<double> scrollOffsetY;
        std::shared_ptr<Slider> vslider;
        std::shared_ptr<Slider> hslider;
        ReyEngine::Size<int> _childBoundingBox;
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    };

}