#pragma once
#include "Control.h"
#include "MathUtils.h"

namespace ReyEngine{
   class Layout : public Widget {
   public:
      REYENGINE_OBJECT(Layout)
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class LayoutDir{HORIZONTAL, VERTICAL, GRID, OTHER};
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
      void _on_rect_changed() override {arrangeChildren();}
      void _on_child_rect_changed(Widget*) override {arrangeChildren();}
      void render2DEnd() override;
      virtual void arrangeChildren();
      void render2D() const override {drawRectangleLines(getRect().toSizeRect(), 1.0, Colors::black);}
      const LayoutDir layoutDir;

   };
}