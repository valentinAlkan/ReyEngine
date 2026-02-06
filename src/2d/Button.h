#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Button : public Widget {
      enum class DrawState {UP, DOWN_PRESS, DOWN};
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
      }
      void setDown(bool newDown, PublishType pubTybe=PublishType::DO_PUBLISH);
      [[nodiscard]] bool getDown() const;
      void setIsToggle(bool isToggle) { _isToggle = isToggle;}
      [[nodiscard]] bool getIsToggleButton() const {return _isToggle;}
   protected:
      DrawState _drawState = DrawState::UP;
      bool _down = false;
      using ColorPack = std::tuple<ColorRGBA, ColorRGBA, ColorRGBA>;
      Button(const std::string& text) {
         setText(text);
      }
      void _init() override {
         setMinSize(measureText(text, getTheme().font) + Size<float>(10, 10));
      }
      void _applyTheme(const ColorPack& colorPack) {
         theme->background.colorPrimary = std::get<0>(colorPack);
         theme->background.colorSecondary = std::get<1>(colorPack);
         theme->background.colorTertiary = std::get<2>(colorPack);
      }
      void _render2D() const;
      virtual void _on_down_publish() {publish<ButtonDownEvent>(ButtonDownEvent(this));}
      virtual void _on_up_publish(bool mouseEscaped) {if (!mouseEscaped) publish<ButtonUpEvent>(ButtonUpEvent(this, mouseEscaped));}
      void __down();
      void __up();
      Widget* _unhandled_input(const InputEvent& event) override;
      std::string text;
      bool _isToggle = false;
      ColorPack PUSHBUTTON_COLORS = {Colors::gray, Colors::lightGray, Colors::blue};
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   class PushButton : public Button {
   public:
      REYENGINE_OBJECT(PushButton)
      //an up that was inside the button
      EVENT(ButtonPressEvent, 11111114){}};
      PushButton(const std::string& text="Push Button"): Button(text){}
   protected:
      void _init() override {
         Button::_init();
      }
      void _on_up_publish(bool mouseEscaped) override;
      void render2D() const override{_render2D();}
   };

   class ToggleButton : public Button{
   public:
      REYENGINE_OBJECT(ToggleButton)
      EVENT(ButtonToggleEvent, 11111113){}
         [[nodiscard]] const Button& button() const {return static_cast<const Button&>(*publisher);}
      };
      ToggleButton(const std::string& text = "Toggle Button"): Button(text)
      {
         _isToggle = true;
      }
   protected:
      void _init() override {
         Button::_init();
      }
      void _on_up_publish(bool mouseEscaped) override;
      void _on_down_publish() override;
      void render2D() const override{_render2D();}
   };

   class CheckBox : public ToggleButton {
   public:
      REYENGINE_OBJECT(CheckBox)
      CheckBox(const std::string& text): ToggleButton(text){}
      [[nodiscard]] bool getChecked() const {return getDown();} //alias
      void setChecked(bool checked){setDown(checked);}
   protected:
      void render2D() const override;
      void _init() override {
         ReyEngine::ToggleButton::_init();
      }
      void _on_rect_changed() override;
      Rect<float> _box;
      Rect<float> _outline;
      Rect<float> _textRect;
   };
}