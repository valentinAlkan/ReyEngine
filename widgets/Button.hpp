#pragma once
#include "Control.hpp"

class BaseButton : public Control {
   GFCSDRAW_OBJECT(BaseButton, Control)
   , PROPERTY_DECLARE(down){
      _rect.value = GFCSDraw::Rect<double>(0,0,200,50);
   }
public:
   void registerProperties() override{
      registerProperty(down);
   };
   BoolProperty down;
};

class PushButton : public BaseButton{
   GFCSDRAW_OBJECT(PushButton, BaseButton)
   , PROPERTY_DECLARE(text){
      text.value = "PushButton";
   }
   void registerProperties() override {
      BaseButton::registerProperties();
      registerProperty(text);
   };
   void render() const override {
      GFCSDraw::Rect<float> rec = {GFCSDraw::Vec2<float>(0,0), GFCSDraw::Vec2<float>(_rect.value.size())};
      _drawRectangleRoundedLines(rec, 2, 1, 1, BLACK);
      _drawTextCentered(text.value, rec.center(), 20, BLACK);
   }
   StringProperty text;
public:
   std::string getText(){return text.value;}
   void setText(const std::string& newText){text.value = newText;}
protected:

};

