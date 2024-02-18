#pragma once
#include "BaseWidget.h"
#include <string>
#include <utility>
#include "Control.h"

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

class PosTestWidget : public BaseWidget {
public:
   REYENGINE_OBJECT(PosTestWidget, BaseWidget), someString("someString"){}
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;
   TestProperty someString;
};