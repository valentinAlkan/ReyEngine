#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Button : public Widget {
   public:
      //emitted when button is "pressed" (ie released on the button itself)
      EVENT_ARGS(ButtonPressEvent, 11111112, bool down)
      , down(down)
      {}
         bool down;
      };
      //emitted everytime the state of the button is changed, regardless of where the mouse is
      EVENT_ARGS(ButtonToggleEvent, 11111112, bool down, bool mouseEscaped)
      , down(down)
      , mouseEscaped(mouseEscaped)
      {}
         bool down;
         //whether or not the mouse was actually in the button's rect.
         // useful for ignoring an up event if the user dragged the mouse away (which is typical
         // when the user wants that input ignored)
         bool mouseEscaped;
      };
   public:
      bool down = false;
   protected:
      Widget* _unhandled_input(const InputEvent&) override;
      void setDown(bool newDown, bool noEvents=false);
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class PushButton : public Button{
   public:
      explicit PushButton(const std::string& text="Push Button")
      : text(text)
      {
         acceptsHover = true;
         theme->background.colorPrimary = COLORS::gray;
         theme->background.colorSecondary = COLORS::lightGray;
         theme->background.colorTertiary = COLORS::blue;
      }
      void render2D() const override;
      std::string text;
   public:
      std::string getText(){return text;}
      void setText(const std::string& newText){
         text = newText;
         setMinSize(measureText(newText, getTheme().font));
      }
   protected:

   };

}