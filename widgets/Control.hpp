#pragma once
#include "BaseWidget.h"

class Control : public BaseWidget {
public:
//   static std::shared_ptr<BaseWidget> Control::deserialize(const std::string &instanceName,
//                                                           PropertyPrototypeMap &properties) {
//      __CTOR_RECT__ = {0,0,0,0};
//      auto retval = std::make_shared<Control>(instanceName, r);
//      retval->BaseWidget::_deserialize(properties);
//      return retval;
//   }
//   Control(std::string name, GFCSDraw::Rect<float> r):
//   Control(std::move(name),"Control",r){}
//protected:
//   Control(std::string name, std::string typeName, __CTOR_RECT__):
//   BaseWidget(name, std::move(typeName), r){}
   GFCSDRAW_OBJECT(Control, BaseWidget){}
public:
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
};