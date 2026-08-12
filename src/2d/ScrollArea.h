#pragma once
#include "Widget.h"
#include "Slider.h"
#include "ScrollView.h"

namespace ReyEngine {
   class ScrollArea;

   namespace Internal {
      // The non-scrolling window onto the content. Owns the clip region and rejects input
      // that falls outside it, so what you can click always matches what you can see.
      // Sliders are siblings of this, not children, so they are neither clipped nor scrolled.
      class ScrollAreaViewport : public Widget {
      public:
         REYENGINE_OBJECT(ScrollAreaViewport)
         void render2D(RenderContext&) const override {}
      protected:
         //clip descendants to our own rect. This node never moves, so the clip stays put
         //while the content scrolls underneath it.
         std::optional<Rect<R_FLOAT>> getChildClipRect() const override {return getSizeRect();}
         Handled _processInput(const InputEvent&) override;
      };

      // Holds the scrolled children, positioned at -offset. Making the offset a real
      // widget position is the whole trick: render, input and getGlobalTransform all walk
      // the transform chain already, so none of them need to know about scrolling.
      class ScrollAreaContent : public Widget {
      public:
         REYENGINE_OBJECT(ScrollAreaContent)
         explicit ScrollAreaContent(ScrollArea& owner) : _owner(owner) {}
         void render2D(RenderContext&) const override {}
      protected:
         //A grandchild's resize only ever notifies its *closest* parent widget, which is us
         //rather than the ScrollArea. Relay upward or the scroll extents go stale.
         void _on_child_rect_changed(Widget*) override;
         void _on_child_added_to_tree(TypeNode*) override;
      private:
         ScrollArea& _owner;
      };
   }

   /// A scrolling container.
   ///
   /// Children added with make_child(scrollArea, ...) land in an internal content node and
   /// scroll; the scrollbars do not. Clipping is done with scissor regions rather than an
   /// offscreen render target, and the scroll offset lives in the ordinary transform chain,
   /// so hit-testing and coordinate conversion need no special cases.
   ///
   /// Internal structure:
   ///   ScrollArea
   ///   |- __viewport   fixed, clips its subtree
   ///   |  \- __content moves by -offset, holds the children you add
   ///   |- __vslider
   ///   \- __hslider
   class ScrollArea : public Widget {
   public:
      REYENGINE_OBJECT(ScrollArea);
      ScrollArea();
      void hideVSlider(bool hidden);
      void hideHSlider(bool hidden);
      void setOffsetX(const Percent& pct);
      void setOffsetY(const Percent& pct);
      void setOffsetX(float amt);
      void setOffsetY(float amt);
      [[nodiscard]] Pos<R_FLOAT> getScrollOffset() const {return _scroll.offset();}
      [[nodiscard]] Size<R_FLOAT> getViewportSize() const;

      //the node our children actually live in - see TreeStorable::getChildInsertionNode
      TypeNode* getChildInsertionNode() override;

   protected:
      static constexpr std::string_view VIEWPORT_NAME = "__viewport";
      static constexpr std::string_view CONTENT_NAME = "__content";
      static constexpr std::string_view VSLIDER_NAME = "__vslider";
      static constexpr std::string_view HSLIDER_NAME = "__hslider";

      void render2D(RenderContext&) const override;
      void _process(float dt) override {}
      void _on_rect_changed() override;
      void _on_child_rect_changed(Widget*) override;
      Handled _unhandled_input(const InputEvent&) override;
      Handled _processInput(const InputEvent&) override;
      void _init() override;

      /// Recompute content extents, bar visibility/geometry and the viewport, then re-apply
      /// the offset. Safe to call redundantly; re-entry is ignored.
      void refreshScroll();

      std::shared_ptr<Internal::ScrollAreaViewport> _viewport;
      std::shared_ptr<Internal::ScrollAreaContent> _content;
      std::shared_ptr<Slider> _vslider;
      std::shared_ptr<Slider> _hslider;
      ScrollView _scroll;
      bool _hideVSlider = false;
      bool _hideHSlider = false;

   private:
      void applyOffset(); //push the model's offset onto the content node's position
      static constexpr float sliderSize = 20;
      //refreshScroll resizes and repositions its own children, and those changes come back
      //to us as child-rect notifications. Without this it would recurse forever.
      bool _refreshing = false;

      friend class Internal::ScrollAreaContent;
   };
}
