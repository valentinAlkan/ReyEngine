#pragma once
#include "Control.hpp"

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text){
      setText("Label");
   }
public:
   void render() const override{
      //todo: scissor text
      _drawText(text.value, {0,0}, 20, BLACK);
   };
   void _process(float dt) override {};
   void registerProperties() override{
      registerProperty(text);
   };
   void setText(const std::string& newText){
      text.set(newText);
//      auto textSize = GFCSDraw::measureText(GFCSDraw::getDefaultFont(), text.get().c_str(), 20, 1);
      auto textWidth = MeasureText("yo", 20);
      setRect({0,0,(double)textWidth, 20});
   }
protected:
   StringProperty text;
};
