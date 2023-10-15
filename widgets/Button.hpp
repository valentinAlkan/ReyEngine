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
protected:
   Handled _unhandled_input(InputEvent& event) override{
      auto& e = static_cast<InputEventMouseButton&>(event);
      if (e.button == InputInterface::MouseButton::MOUSE_BUTTON_LEFT){
         down.set(e.isDown);
         return true;
      }
      return false;
   }
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
      static constexpr int SEGMENTS = 10;
      static constexpr int THICKNESS = 2;
      GFCSDraw::Rect<float> rec = {GFCSDraw::Vec2<float>(0,0), GFCSDraw::Vec2<float>(_rect.value.size())};
      _drawRectangleRounded(rec, 2, SEGMENTS, down.value ? GRAY : LIGHTGRAY);
      _drawRectangleRoundedLines(rec, 2, SEGMENTS, THICKNESS, BLACK);
      _drawTextCentered(text.value, rec.center(), 20, BLACK);
   }
   StringProperty text;
public:
   std::string getText(){return text.value;}
   void setText(const std::string& newText){text.value = newText;}
protected:

};

