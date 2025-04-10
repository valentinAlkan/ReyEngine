#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Button : public Widget {
   public:
      // when the button is down
      EVENT(ButtonDownEvent, 11111111){}};
      EVENT_ARGS(ButtonUpEvent, 11111112, bool mouseEscaped)
      , mouseEscaped(mouseEscaped)
      {}
         //whether or not the mouse was actually in the button's rect.
         // useful for ignoring an up event if the user dragged the mouse away (which is typical
         // when the user wants that input ignored)
         const bool mouseEscaped;
      };
   //an up that was inside the button
   EVENT(ButtonPressEvent, 11111113){}};
   public:
      bool down = false;
      [[nodiscard]] std::string getText() const {return text;}
      void setText(const std::string& newText){
         text = newText;
         setMinSize(measureText(newText, getTheme().font) + Size<float>(10,10));
      }
      void setDown(bool newDown, bool noEvents=false);
   protected:
      std::string text;
      Widget* _unhandled_input(const InputEvent&) override;
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class PushButton : public Button{
   public:
      explicit PushButton(const std::string& text="Push Button")
      {
         setText(text);
         acceptsHover = true;
         theme->background.colorPrimary = COLORS::gray;
         theme->background.colorSecondary = COLORS::lightGray;
         theme->background.colorTertiary = COLORS::blue;
      }
      void render2D() const override;

   public:
   protected:
      void _on_rect_changed() override {};
   };

}