#pragma once
#include "Control.h"
#include "MathTools.h"

namespace ReyEngine{
   class Layout : public BaseWidget {
   public:
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class LayoutDir{HORIZONTAL, VERTICAL, GRID, OTHER};
      /////////////////////////////////////////////////////////////////////////////////////////
      struct LayoutProperty : public EnumProperty<LayoutDir, 3>{
         LayoutProperty(const std::string& instanceName,  LayoutDir defaultvalue)
               : EnumProperty<LayoutDir, 3>(instanceName, std::move(defaultvalue)){}
         const EnumPair<LayoutDir, 3>& getDict() const override {return dict;}
         static constexpr EnumPair<LayoutDir, 3> dict = {
               ENUM_PAIR_DECLARE(LayoutDir, VERTICAL),
               ENUM_PAIR_DECLARE(LayoutDir, HORIZONTAL),
               ENUM_PAIR_DECLARE(LayoutDir, GRID),
         };
         void registerProperties() override {}
      };
       ReyEngine::Size<int> calculateIdealBoundingBox();
      /////////////////////////////////////////////////////////////////////////////////////////
   protected:
      Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir);
      void _register_parent_properties() override;
      void _on_child_added(std::shared_ptr<BaseWidget>& child) override;
   //   void _on_child_added_immediate(std::shared_ptr<BaseWidget>& child) override;
      void _on_child_removed(std::shared_ptr<BaseWidget>& child) override;
      void _on_rect_changed() override {arrangeChildren();}
      void renderEnd() override;
      virtual void arrangeChildren();
      void render() const override {};
      //Given infinite layout space and children of minimum possible sizes, what would the childBoundingBox for this layout be?
      // Useful to predict the rect of common layout situations - particularly pop up menus and the like.
   public:
      FloatListProperty childScales;
   protected:
      const LayoutDir dir;
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class VLayout : public Layout {
   public:
      VLayout(const std::string& instanceName)
      : Layout(instanceName, _get_static_constexpr_typename(), LayoutDir::VERTICAL)
      {}
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(VLayout);
      REYENGINE_SERIALIZER(VLayout, Layout)

   protected:
      VLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::VERTICAL){}
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class HLayout : public Layout {
   public:
      HLayout(const std::string& instanceName)
      : Layout(instanceName, _get_static_constexpr_typename(), Layout::LayoutDir::HORIZONTAL)
      {}
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(HLayout);
      REYENGINE_SERIALIZER(HLayout, Layout)
   protected:
      HLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::HORIZONTAL){}
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class GridLayout : public Layout {
   public:
       GridLayout(const std::string& instanceName)
       : Layout(instanceName, _get_static_constexpr_typename(), Layout::LayoutDir::GRID)
       {}
       REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(GridLayout);
       REYENGINE_SERIALIZER(GridLayout, Layout)
   protected:
       GridLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::GRID){}
   };
}