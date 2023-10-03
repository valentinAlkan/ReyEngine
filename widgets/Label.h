#pragma once
#include "BaseWidget.h"

class Label : public BaseWidget {
public:
   Label(std::string name);
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;
   static std::shared_ptr<Label> deserialize(std::string instanceName, PropertyPrototypeMap);

protected:
   StringProperty text;
};
