#pragma once
#include "Control.hpp"

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text){
      text.value = "Label";
   }
public:
   void render() const override{
      _drawText(text.value, {0,0}, 20, BLACK);
   };
   void _process(float dt) override {};
   void registerProperties() override{
      registerProperty(text);
   };
   void setText(const std::string& newText){
      text.set(newText);
   }
protected:
   StringProperty text;
};
