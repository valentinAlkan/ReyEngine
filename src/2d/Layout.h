#pragma once
#include "Control.h"
#include "MathUtils.h"

namespace ReyEngine{
   class Layout : public Widget {
   public:
      REYENGINE_OBJECT(Layout)
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class LayoutDir{HORIZONTAL, VERTICAL, GRID, NONE}; //None is reserved for use by subclasses
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class Alignment{EVEN, FRONT, BACK};

      Layout(Layout::LayoutDir layoutDir);
      ReyEngine::Size<int> calculateIdealBoundingBox();
      std::vector<float> layoutRatios;
      Alignment alignment;
      /////////////////////////////////////////////////////////////////////////////////////////
   protected:
      void layoutApplyRect(Widget* widget, Rect<float>& r);
      struct LayoutHelper;
      void _on_child_added_to_tree(TypeNode*) override;
      void _on_child_removed_from_tree(TypeNode*) override;
      void _on_rect_changed() override;
      void _on_child_rect_changed(Widget*) override {arrangeChildren();}
      virtual void arrangeChildren();
      void render2D() const override;
      const LayoutDir layoutDir;
      bool _usesLayoutRatios = false; //different types of layouts might not use layout ratios.
      Rect<float> _layoutArea; // the area to use as layout space.
   private:
      ColorRGBA DEBUG_COLOR;

   };
}