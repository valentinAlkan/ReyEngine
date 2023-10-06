#pragma once
#include "BaseWidget.h"

class Control : public BaseWidget {
   GFCSDRAW_OBJECT(Control, BaseWidget){}
public:
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{}
};