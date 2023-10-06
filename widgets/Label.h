#pragma once
#include "BaseWidget.h"

class Label : public BaseWidget {
   GFCSDRAW_OBJECT(Label, BaseWidget)
public:
   Label(std::string name);
   void render() const override;
   void _process(float dt) override;
   void registerProperties() override;

protected:
   StringProperty text;
};
