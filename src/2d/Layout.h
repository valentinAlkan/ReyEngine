#pragma once
#include "Control.h"
#include "MathUtils.h"

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

      enum class Alignment{EVEN, FRONT, BACK};
      /////////////////////////////////////////////////////////////////////////////////////////
      struct AlignmentProperty : public EnumProperty<Alignment, 3>{
         AlignmentProperty(const std::string& instanceName,  Alignment defaultvalue)
         : EnumProperty<Alignment, 3>(instanceName, std::move(defaultvalue)){}
         const EnumPair<Alignment, 3>& getDict() const override {return dict;}
         static constexpr EnumPair<Alignment, 3> dict = {
            ENUM_PAIR_DECLARE(Alignment, EVEN),
            ENUM_PAIR_DECLARE(Alignment, FRONT),
            ENUM_PAIR_DECLARE(Alignment, BACK),
         };
         void registerProperties() override {}
      };

      ReyEngine::Size<int> calculateIdealBoundingBox();
      FloatListProperty layoutRatios;
      AlignmentProperty alignment;
      /////////////////////////////////////////////////////////////////////////////////////////
   protected:
      struct LayoutHelper;
      Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir);
      void _register_parent_properties() override;
      void _on_child_added(std::shared_ptr<BaseWidget>& child) override;
      void _on_child_removed(std::shared_ptr<BaseWidget>& child) override;
      void _on_rect_changed() override {arrangeChildren();}
      void renderEnd() override;
      virtual void arrangeChildren();
      void render() const override {};
      const LayoutDir dir;

   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class VLayout : public Layout {
   public:
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(VLayout);
      REYENGINE_SERIALIZER(VLayout, Layout)
      REYENGINE_DEFAULT_BUILD(VLayout);
   protected:
      VLayout(const std::string& instanceName)
      : Layout(instanceName, _get_static_constexpr_typename(), LayoutDir::VERTICAL)
      , NamedInstance(instanceName, _get_static_constexpr_typename())
      , Component(instanceName, _get_static_constexpr_typename()){}
      VLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::VERTICAL)
      , NamedInstance(name, _get_static_constexpr_typename()), Internal::Component(name, _get_static_constexpr_typename()){}
   private:
      REYENGINE_PRIVATE_MAKE_SHARED(VLayout)
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class HLayout : public Layout {
   public:
      REYENGINE_DEFAULT_BUILD(HLayout);
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(HLayout);
      REYENGINE_SERIALIZER(HLayout, Layout)
   protected:
      HLayout(const std::string& instanceName)
      : Layout(instanceName, _get_static_constexpr_typename(), Layout::LayoutDir::HORIZONTAL)
      , NamedInstance(instanceName, _get_static_constexpr_typename())
      , Component(instanceName, _get_static_constexpr_typename())
      {}
      HLayout(const std::string& instanceName, const std::string& typeName): Layout(instanceName, typeName, LayoutDir::HORIZONTAL)
      , NamedInstance(instanceName, typeName)
      , Component(instanceName, _get_static_constexpr_typename())
      {}
   private:
      REYENGINE_PRIVATE_MAKE_SHARED(HLayout)
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class GridLayout : public Layout {
   public:
       REYENGINE_DEFAULT_BUILD(GridLayout);
       REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(GridLayout);
       REYENGINE_SERIALIZER(GridLayout, Layout)
   protected:
       GridLayout(const std::string& instanceName)
       : Layout(instanceName, _get_static_constexpr_typename(), Layout::LayoutDir::HORIZONTAL)
       , NamedInstance(instanceName, _get_static_constexpr_typename())
       , Component(instanceName, _get_static_constexpr_typename())
       {}
       GridLayout(const std::string& instanceName, const std::string& typeName)
       : Layout(instanceName, typeName, LayoutDir::GRID)
       , NamedInstance(instanceName, typeName)
       , Component(instanceName, _get_static_constexpr_typename())
       {}
   private:
      REYENGINE_PRIVATE_MAKE_SHARED(GridLayout)
   };
}