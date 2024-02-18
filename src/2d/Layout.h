#pragma once
#include "Control.h"
#include "MathTools.h"

class Layout : public Control {
public:
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class LayoutDir{HORIZONTAL, VERTICAL};
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
   void _on_rect_changed() override;
   void renderEnd() override;
   void arrangeChildren();
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
   static constexpr char TYPE_NAME[] = "VLayout";
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      auto retval = std::make_shared<VLayout>(instanceName);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
   VLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::VERTICAL){}
};

/////////////////////////////////////////////////////////////////////////////////////////
class HLayout : public Layout {
public:
   HLayout(const std::string& instanceName)
   : Layout(instanceName, _get_static_constexpr_typename(), Layout::LayoutDir::HORIZONTAL)
   {}
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      auto retval = std::make_shared<HLayout>(instanceName);
      retval->BaseWidget::_deserialize(properties);
   return retval;
}
   static constexpr char TYPE_NAME[] = "HLayout";
protected:
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
   HLayout(const std::string& name, const std::string& typeName): Layout(name, typeName, LayoutDir::VERTICAL){}
};