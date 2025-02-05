#pragma once
#include "Canvas.h"
#include "Slider.hpp"

namespace ReyEngine {
    class ScrollArea : public BaseWidget {
    REYENGINE_OBJECT_BUILD_ONLY(ScrollArea, BaseWidget), scrollOffsetX(0, 0, 0), scrollOffsetY(0, 0, 0) {
        _inputFilter = InputFilter::INPUT_FILTER_PROCESS_AND_STOP; //catch all input so we can prioritize sliders
    }

    public:
        REYENGINE_DEFAULT_BUILD(ScrollArea)
        inline ReyEngine::Size<R_FLOAT> getScrollAreaSize() const {
            return {scrollOffsetX.getMax(), scrollOffsetY.getMax()};
        }

        inline ReyEngine::Pos<R_FLOAT> getScrollOffset() const {
            return {scrollOffsetX.getValue(), scrollOffsetY.getValue()};
        }

        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
        Pos<R_FLOAT> getViewportSize(){return _viewport.size();};
    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void renderBegin() override;
        void render() const {};
        void renderEnd() override;
        void _process(float dt) override {}

        void registerProperties() override {}

        void _on_rect_changed() override;
        void _on_child_rect_changed(std::shared_ptr<BaseWidget>&) override;
        void _on_child_added(std::shared_ptr<BaseWidget> &child) override;
        void _init() override;
        Handled _unhandled_input (const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
        std::optional<std::shared_ptr<BaseWidget>> askHover(const ReyEngine::Pos<R_FLOAT>& globalPos) override;
//        Handled _unhandled_masked_input(const InputEventMouse&, const std::optional<UnhandledMouseInput>&) override;
        ///Ignores scroll bars, but adds their widths if they are visible
        ReyEngine::Size<R_FLOAT> getScrollAreaChildBoundingBox();

        ReyEngine::Range<R_FLOAT> scrollOffsetX;
        ReyEngine::Range<R_FLOAT> scrollOffsetY;
        std::shared_ptr<Slider> vslider;
        std::shared_ptr<Slider> hslider;
        ReyEngine::Size<R_FLOAT> _childBoundingBox;
        Rect<R_FLOAT> _viewport; //the 'sliding' rectangle that moves over the render target, changing what's in the view
        Rect<R_FLOAT> _window; //the area where we draw what's in the viewport
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    private:
        void updateViewport();
        RenderTarget _renderTarget;
    };

}