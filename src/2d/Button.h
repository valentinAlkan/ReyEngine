#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class BaseButton : public BaseWidget {
   public:
      //emitted when button is "pressed" (ie released on the button itself)
      struct ButtonPressEvent : public Event<ButtonPressEvent>{
         EVENT_CTOR_SIMPLE(ButtonPressEvent, Event<ButtonPressEvent>, bool down), down(down){}
         bool down;
      };
      //emitted everytime the state of the button is changed, regardless of where the mouse is
      struct ButtonToggleEvent : public Event<ButtonToggleEvent>{
         EVENT_CTOR_SIMPLE(ButtonToggleEvent, Event<ButtonToggleEvent>, bool down, bool mouseCapture), down(down){}
         bool down;
         //whether or not the mouse was actually in the button's rect.
         // useful for ignoring an up event if the user dragged the mouse away (which is typical
         // when the user wants that input ignored)
         bool mouseCapture;
      };
   protected:
      REYENGINE_VIRTUAL_OBJECT(BaseButton, BaseWidget)
      , PROPERTY_DECLARE(down)
      {
         acceptsHover = true;
      }
   public:
      void registerProperties() override{
         registerProperty(down);
      };
      BoolProperty down;
   //   bool wasDown;
   protected:
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void setDown(bool newDown);
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class PushButton : public BaseButton{
   REYENGINE_OBJECT_CUSTOM_BUILD(PushButton, BaseButton, std::tuple<const std::string&>, std::tuple<const std::string&, const std::string&>)
      //must implement all necessary constructors yourself
      PushButton(const std::string& name)
      : REYENGINE_CTOR_INIT_LIST(name, BaseButton)
      , PROPERTY_DECLARE(text)
      {
         theme->background.colorPrimary.set(COLORS::gray);
         theme->background.colorSecondary.set(COLORS::lightGray);
         theme->background.colorTertiary.set(COLORS::blue);
      }
      void registerProperties() override {
         registerProperty(text);
      };
      void render2D() const override;
      StringProperty text;
   public:
      static std::shared_ptr<PushButton> build(const std::string& name){return PushButton::build(name, name);}
      static std::shared_ptr<PushButton> build(const std::string& name, const std::string& text){
         auto ptr = _reyengine_make_shared(name);
         ptr->setText(text);
         return ptr;
      }
      std::string getText(){return text.value;}
      void setText(const std::string& newText){
         text.value = newText;
         setMinSize(measureText(newText, getTheme()->font));
      }
   protected:

   };

}