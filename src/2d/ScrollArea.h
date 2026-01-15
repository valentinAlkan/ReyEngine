#pragma once
#include "Canvas.h"
#include "Slider.h"

namespace ReyEngine {
    class ScrollArea : public Canvas {
    public:
        REYENGINE_OBJECT(ScrollArea);
        ScrollArea();
        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
    protected:
        void render2D() const override;
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void _process(float dt) override {}

        void _on_rect_changed() override;
        void _on_child_rect_changed(Widget*) override;
        void _on_child_added_to_tree(TypeNode*) override;
        Widget* _unhandled_input(const InputEvent&) override;
        void _init() override;

        Percent scrollOffsetX;
        Percent scrollOffsetY;
        std::shared_ptr<Slider> _vslider;
        std::shared_ptr<Slider> _hslider;
        Rect<R_FLOAT> boundingBox; //contains all children, is at least as big as scrollArea. RenderTarget takes its size
        Rect<R_FLOAT> _viewport;
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    private:
        static constexpr float sliderSize = 20;
        bool _autoResize = true; // will cause the internal render target to resize automatically to fit children outside of the extents of the scroll area
    };

}