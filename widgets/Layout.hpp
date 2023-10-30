#pragma once
#include "Control.hpp"

//class Layout : public Control {
//   enum class LayoutDir{HORIZONTAL, VERTICAL};
//   GFCSDRAW_OBJECT(Layout, Control){}
//   void _on_rect_changed() override {
//
//   }
//   void registerProperties() override{
//      //register properties specific to your type here.
//   }
//};

class Layout : public Control {
public:
   static constexpr char TYPE_NAME[] = "Layout";
   std::string _get_static_constexpr_typename() override{ return TYPE_NAME;}
public:
//   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
//      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
//      auto retval = std::make_shared<Layout>(instanceName, r);
//      retval->BaseWidget::_deserialize(properties);
//   return retval;
//}
protected:
   Layout(const std::string &name, const GFCSDraw::Rect<float> &r) : Layout(name, _get_static_constexpr_typename(), r) {}
   void _register_parent_properties() override{
      Control::_register_parent_properties();
      Control::registerProperties();
   }
   Layout(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float> &r) : Control(name, typeName, r)
   , layoutRatios("layoutRatios")
   {}
   ListProperty<float> layoutRatios;
};