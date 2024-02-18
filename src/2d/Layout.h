#pragma once
#include "Control.h"
#include "MathTools.h"

class Layout : public BaseWidget {
public:
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class LayoutDir{HORIZONTAL, VERTICAL, OTHER};
   /////////////////////////////////////////////////////////////////////////////////////////
   struct LayoutProperty : public EnumProperty<LayoutDir, 2>{
      LayoutProperty(const std::string& instanceName,  LayoutDir defaultvalue)
            : EnumProperty<LayoutDir, 2>(instanceName, defaultvalue){}
      const EnumPair<LayoutDir, 2>& getDict() const override {return dict;}
      static constexpr EnumPair<LayoutDir, 2> dict = {
            ENUM_PAIR_DECLARE(LayoutDir, VERTICAL),
            ENUM_PAIR_DECLARE(LayoutDir, HORIZONTAL),
      };
      void registerProperties() override {}
   };
   /////////////////////////////////////////////////////////////////////////////////////////
protected:
   Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir);
   void _register_parent_properties() override;
   void _on_child_added(std::shared_ptr<BaseWidget>& child) override;
   void _on_child_added_immediate(std::shared_ptr<BaseWidget>& child) override;
   void _on_rect_changed() override {arrangeChildren();}
   void renderEnd() override;
   virtual void arrangeChildren();
   void render() const override {};
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
   HLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::VERTICAL){}
};