#pragma once
#include "BaseWidget.h"
#include <string>
#include <utility>
#include "Control.hpp"

struct TestProperty : StringProperty{
   TestProperty(const std::string& instanceName): StringProperty(instanceName)
   , subProp("subProp")
   {
      subProp.set("default");
   }
   void registerProperties() override {
      registerProperty(subProp);
   }
   StringProperty subProp;
};

class PosTestWidget : public Control {
protected:
   static constexpr char DEFAULT_TEXT[] = "someString";
   PosTestWidget(std::string name, std::string typeName)
   : Control(std::move(name), std::move(typeName))
   , someString(DEFAULT_TEXT)
   {}
public:
   GFCSDRAW_OBJECT(PosTestWidget, Control)
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;
   TestProperty someString;
};
