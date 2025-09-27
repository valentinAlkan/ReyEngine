#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Button : public Widget {
   public:
      REYENGINE_OBJECT(Button)
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

   public:
      void click(); // a down followed by an up.
      [[nodiscard]] std::string getText() const {return text;}
      void setText(const std::string& newText){
         text = newText;
         setMinSize(measureText(newText, getTheme().font) + Size<float>(10,10));
      }
      void setDown(bool newDown, PublishType pubTybe=PublishType::DO_PUBLISH);
      [[nodiscard]] bool getDown(){return down;}
      void setToggle(bool isToggle){_toggle = isToggle;}
      [[nodiscard]] bool getToggle(){return _toggle;}
   protected:
      bool down = false;
      using ColorPack = std::tuple<ColorRGBA, ColorRGBA, ColorRGBA>;
      Button(const std::string& text) {
         setText(text);
         acceptsHover = true;
      }
      void _applyTheme(const ColorPack& colorPack) {
         theme->background.colorPrimary = std::get<0>(colorPack);
         theme->background.colorSecondary = std::get<1>(colorPack);
         theme->background.colorTertiary = std::get<2>(colorPack);
      }
      void _render2D() const;
      virtual void _on_down() {publish<ButtonDownEvent>(ButtonDownEvent(this));}
      virtual void _on_up(bool mouseEscaped) {publish<ButtonUpEvent>(ButtonUpEvent(this, mouseEscaped));}
      Widget* _unhandled_input(const InputEvent& event);
      std::string text;
      bool _toggle = false;
       ColorPack PUSHBUTTON_COLORS = {Colors::gray, Colors::lightGray, Colors::blue};
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class ToggleButton : public Button{
   public:
      REYENGINE_OBJECT(ToggleButton)
      EVENT(ButtonToggleEvent, 11111113){}};
      ToggleButton(const std::string& text): Button(text)
      {
         _applyTheme(PUSHBUTTON_COLORS);
      }
   protected:
      void _on_up(bool mouseEscaped) override;
      void _on_down() override;
      void render2D() const override{_render2D();}
   };
   class PushButton : public Button {
   public:
      REYENGINE_OBJECT(PushButton)
      //an up that was inside the button
      EVENT(ButtonPressEvent, 11111114){}};
      PushButton(const std::string& text="Push Button")
      : Button(text)
      {
         _applyTheme(PUSHBUTTON_COLORS);
      }
   protected:
      void _on_up(bool mouseEscaped) override;
      void render2D() const override{_render2D();}
   };

}