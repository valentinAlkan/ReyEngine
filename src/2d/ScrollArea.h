#pragma once
#include "Canvas.h"
#include "Slider.h"

namespace ReyEngine {
    class ScrollArea : public Canvas {
    public:
        ScrollArea();
        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
        Rect<R_FLOAT> getViewPort(){return _viewport;}
        Rect<R_FLOAT> getRenderBox(){return boundingBox;}
    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void render2DBegin() override;
//        void render2D() const;
        void render2DEnd() override;
        void _process(float dt) override {}

        void _on_rect_changed() override;
        void _on_child_rect_changed(Widget*) override;
//        void _on_child_added_to_tree(TypeNode* child) override;
        void _init() override;
        Widget* _unhandled_input (const InputEvent&) override;
//        std::optional<Widget*> askHover(const Pos<R_FLOAT>& globalPos) override;

        Percent scrollOffsetX;
        Percent scrollOffsetY;
        Slider* vslider;
        Slider* hslider;
        Rect<R_FLOAT> boundingBox; //contains all children, is at least as big as scrollArea. RenderTarget takes its size
        //_viewport and window should always be the same size, but have different positions
        Rect<R_FLOAT> _window; //the area where we draw the rect (size of the scrollArea widget - sliders)
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    private:
        static constexpr float sliderSize = 20;
        void updateViewport();
    };

}