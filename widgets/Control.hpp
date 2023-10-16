#pragma once
#include "BaseWidget.h"

class Control : public BaseWidget {
   GFCSDRAW_OBJECT(Control, BaseWidget){}
public:
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{
      // YOU MUST CALL THE PARENT registerProperties FUNCTION! OR YOU DIE!
      // It is good practice to override it and call the parent even
      // if you have no properties to register just so you dont forget
      BaseWidget::registerProperties();
      //register properties specific to your type here.
   }
};