#pragma once
#include "BaseWidget.h"

class Label : public BaseWidget {
   GFCSDRAW_OBJECT(Label, BaseWidget)
public:
   Label(std::string name)
   : BaseWidget(std::move(name), "Label")
   , text("DefaultText"){}
   void render() const override{
      _drawText(text.value, {0,0}, 20, BLACK);
   };
   void _process(float dt) override {};
   void registerProperties() override{
      registerProperty(text);
   };

protected:
   StringProperty text;
};
