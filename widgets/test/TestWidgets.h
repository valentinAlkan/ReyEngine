#pragma once
#include "BaseWidget.h"
#include <string>

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
   GFCSDRAW_OBJECT(PosTestWidget, BaseWidget);
   PosTestWidget(std::string name);
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;
   TestProperty someString;
};
