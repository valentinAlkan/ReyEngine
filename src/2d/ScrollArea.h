#pragma once
#include "Control.h"
#include "Slider.hpp"

namespace ReyEngine {
    class ScrollArea : public BaseWidget {
    REYENGINE_OBJECT_BUILD_ONLY(ScrollArea, BaseWidget), scrollOffsetX(0, 0, 0), scrollOffsetY(0, 0, 0) {
        _inputFilter = InputFilter::INPUT_FILTER_PROCESS_AND_STOP; //catch all input so we can prioritize sliders
    }

    public:
        REYENGINE_DEFAULT_BUILD(ScrollArea)
        inline ReyEngine::Size<double> getScrollAreaSize() const {
            return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};
        }

        inline ReyEngine::Pos<double> getScrollOffset() const {
            return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};
        }

        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
        Pos<int> getViewportSize(){return _viewport.size();};
    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void renderBegin(ReyEngine::Pos<double> &textureOffset) override;
        void render() const {}
        void renderEnd() override;
        void _process(float dt) override {}

        void registerProperties() override {}

        void _on_rect_changed() override;
        void _on_child_added(std::shared_ptr<BaseWidget> &child) override;
        void _init() override;
        Handled _unhandled_input (const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
        std::optional<std::shared_ptr<BaseWidget>> askHover(const ReyEngine::Pos<int>& globalPos) override;
//        Handled _unhandled_masked_input(const InputEventMouse&, const std::optional<UnhandledMouseInput>&) override;
        ///Ignores scroll bars, but adds their widths if they are visible
        ReyEngine::Size<int> getScrollAreaChildBoundingBox();

        ReyEngine::Range<double> scrollOffsetX;
        ReyEngine::Range<double> scrollOffsetY;
        std::shared_ptr<Slider> vslider;
        std::shared_ptr<Slider> hslider;
        ReyEngine::Size<int> _childBoundingBox;
        Rect<int> _viewport; //the area that isn't obscured by sliders
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    };

}