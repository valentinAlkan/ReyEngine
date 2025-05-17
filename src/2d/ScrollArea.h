#pragma once
#include "Canvas.h"
#include "Slider.h"

namespace ReyEngine {
    class ScrollArea : public Canvas {
    public:
        ScrollArea();
        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void _process(float dt) override {}

        void _on_rect_changed() override;
        void _on_child_rect_changed(Widget*) override;
        void _on_child_added_to_tree(TypeNode*) override;
        void _init() override;

        Percent scrollOffsetX;
        Percent scrollOffsetY;
        Slider* vslider = nullptr;
        Slider* hslider = nullptr;
        Rect<R_FLOAT> boundingBox; //contains all children, is at least as big as scrollArea. RenderTarget takes its size
        Rect<R_FLOAT> _viewport;
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    private:
        static constexpr float sliderSize = 20;
        bool _autoResize = true; // will cause the internal render target to resize automatically to fit children outside of the extents of the scroll area
        void updateViewport();
    };

}