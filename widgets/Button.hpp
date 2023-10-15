#pragma once
#include "Control.hpp"

namespace EventType {
   DECLARE_EVENT(EVENT_PUSHBUTTON)
}

struct PushButtonEvent : public Event{
   PushButtonEvent(bool down)
   : Event(EventType::EVENT_PUSHBUTTON)
   , down(down)
   {}
   bool down;
};

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
   bool wasDown;
protected:
   Handled _unhandled_input(InputEvent& event) override{
      auto& e = static_cast<InputEventMouseButton&>(event);
      if (e.button == InputInterface::MouseButton::MOUSE_BUTTON_LEFT){
         setDown(e.isDown);
         return true;
      }
      return false;
   }
   void setDown(bool newDown){
      if (wasDown != newDown){
         down.set(newDown);
         auto e = std::make_shared<PushButtonEvent>(newDown);
         publishEvent(e);
      }
   }
};

class PushButton : public BaseButton{
   GFCSDRAW_OBJECT(PushButton, BaseButton)
   , PROPERTY_DECLARE(text){
      text.value = getName();
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

