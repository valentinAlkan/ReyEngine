#pragma once
#include "BaseWidget.h"
#include <string>

struct TestProperty : StringProperty{
   TestProperty(const std::string& instanceName): StringProperty(instanceName){}
};

class PosTestWidget : public BaseWidget {
public:
   PosTestWidget(std::string name);
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;
   static std::shared_ptr<PosTestWidget> deserialize(std::string instanceName, PropertyPrototypeMap);

private:
   TestProperty someString;
};
