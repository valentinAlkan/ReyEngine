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
//      struct LayoutProperty : public EnumProperty<LayoutDir, 3>{
//         LayoutProperty(const std::string& instanceName,  LayoutDir defaultvalue)
//         : EnumProperty<LayoutDir, 3>(instanceName, std::move(defaultvalue)){}
//         const EnumPair<LayoutDir, 3>& getDict() const override {return dict;}
//         static constexpr EnumPair<LayoutDir, 3> dict = {
//            ENUM_PAIR_DECLARE(LayoutDir, VERTICAL),
//            ENUM_PAIR_DECLARE(LayoutDir, HORIZONTAL),
//            ENUM_PAIR_DECLARE(LayoutDir, GRID),
//         };
//         void registerProperties() override {}
//      };

      enum class Alignment{EVEN, FRONT, BACK};
      /////////////////////////////////////////////////////////////////////////////////////////
//      struct AlignmentProperty : public EnumProperty<Alignment, 3>{
//         AlignmentProperty(const std::string& instanceName,  Alignment defaultvalue)
//         : EnumProperty<Alignment, 3>(instanceName, std::move(defaultvalue)){}
//         const EnumPair<Alignment, 3>& getDict() const override {return dict;}
//         static constexpr EnumPair<Alignment, 3> dict = {
//            ENUM_PAIR_DECLARE(Alignment, EVEN),
//            ENUM_PAIR_DECLARE(Alignment, FRONT),
//            ENUM_PAIR_DECLARE(Alignment, BACK),
//         };
//         void registerProperties() override {}
//      };

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
      ColorRGBA _debugColor = ColorRGBA::random(255);
      void render2D() const override {drawRectangle(getRect().toSizeRect(), _debugColor);}
      const LayoutDir dir;

   };
}