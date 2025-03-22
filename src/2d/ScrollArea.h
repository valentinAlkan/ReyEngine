#pragma once
#include "Canvas.h"
#include "Slider.h"

namespace ReyEngine {
    class ScrollArea : public Widget {
    public:
        ScrollArea(){
           _inputFilter = InputFilter::INPUT_FILTER_PROCESS_AND_STOP;
        }
        void hideVSlider(bool hidden);
        void hideHSlider(bool hidden);
        Rect<R_FLOAT> getViewPort(){return _viewport;}
        Rect<R_FLOAT> getRenderBox(){return _renderBox;}
    protected:
        static constexpr std::string_view VSLIDER_NAME = "__vslider";
        static constexpr std::string_view HSLIDER_NAME = "__hslider";

        void render2DBegin() override;
        void render2D() const;
        void render2DEnd() override;
        void _process(float dt) override {}

        void _on_rect_changed() override;
        void _on_child_rect_changed(Widget*) override;
        void _on_child_added_to_tree(TypeNode* child) override;
        void _init() override;
        Handled _unhandled_input (const InputEvent&) override;
//        std::optional<Widget*> askHover(const Pos<R_FLOAT>& globalPos) override;

        Percent scrollOffsetX;
        Percent scrollOffsetY;
        Widget* vslider;
        Widget* hslider;
        Rect<R_FLOAT> _renderBox; //contains all children, is at least as big as scrollArea. RenderTarget takes its size
        //viewport and window should always be the same size, but have different positions
        Rect<R_FLOAT> _viewport; //the 'sliding' rectangle that moves over the render target, changing what's in the view
        Rect<R_FLOAT> _window; //the area where we draw the rect (size of the scrollArea widget - sliders
        bool _hideVSlider = false;
        bool _hideHSlider = false;
    private:
        RenderTarget renderTarget;
        Matrix globalMatrix;
        static constexpr float sliderSize = 20;
        void updateViewport();
        RenderTarget _renderTarget;
    };

}