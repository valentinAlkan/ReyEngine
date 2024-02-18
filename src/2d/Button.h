#pragma once
#include "BaseWidget.h"

class BaseButton : public BaseWidget {
public:
   struct ButtonPressEvent : public Event<ButtonPressEvent>{
      EVENT_CTOR_SIMPLE(ButtonPressEvent, Event<ButtonPressEvent>, bool down), down(down){}
      bool down;
   };
protected:
   REYENGINE_VIRTUAL_OBJECT(BaseButton, BaseWidget)
   , PROPERTY_DECLARE(down){
      _rect.value = ReyEngine::Rect<double>(0,0,200,50);
      acceptsHover = true;
   }
public:
   void registerProperties() override{
      registerProperty(down);
   };
   BoolProperty down;
   bool wasDown;
protected:
   Handled _unhandled_input(InputEvent& event) override;
   void setDown(bool newDown);
};

/////////////////////////////////////////////////////////////////////////////////////////
class PushButton : public BaseButton{
   REYENGINE_OBJECT(PushButton, BaseButton)
   , PROPERTY_DECLARE(text){
      text.value = getName();
      theme->background.colorPrimary.set(COLORS::gray);
      theme->background.colorSecondary.set(COLORS::lightGray);
      theme->background.colorTertiary.set(COLORS::blue);
   }
   void registerProperties() override {
      registerProperty(text);
   };
   void render() const override;
   StringProperty text;
public:
   std::string getText(){return text.value;}
   void setText(const std::string& newText){text.value = newText;}
protected:

};

