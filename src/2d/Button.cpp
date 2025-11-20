#include "Button.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Button::_unhandled_input(const InputEvent& event) {
   bool wasDown = down;
   if (!enabled) return nullptr;
   if (event.isEvent<InputEventMouseButton>()) {
      auto& mbEvent = event.toEvent<InputEventMouseButton>();
      if (mbEvent.button == InputInterface::MouseButton::LEFT) {
         bool isInside = event.isMouse().value()->isInside();
         if (isFocused() && wasDown && !mbEvent.isDown) {
           //button is down and it was just released *somewhere*
            down = false;
            setFocused(false);
            _on_up(isInside);
            return this;
         } else if (mbEvent.isDown && isInside) {
           //normal inside-click
            down = true;
            setFocused(true);
            _on_down();
            return this;
         }
      }
   }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::setDown(bool newDown, PublishType pubTybe){
   bool wasDown = down;
   down = newDown;
   if (pubTybe == PublishType::DO_PUBLISH) {
      if (down && !wasDown) {
         _on_down();
      } else if (!down && wasDown){
         _on_up(false);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::click() {
   setDown(true, PublishType::DO_PUBLISH);
   setDown(false, PublishType::DO_PUBLISH);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::_render2D() const {
    static constexpr int SEGMENTS = 10;
    static constexpr int THICKNESS = 2;
    ColorRGBA backgroundColor;
    ColorRGBA textColor;
    if (enabled) {
       backgroundColor = theme->background.colorPrimary;
       textColor = theme->font->color;
       if (down) {
          backgroundColor = theme->background.colorTertiary;
       } else if (isHovered()){
          backgroundColor = theme->background.colorSecondary;
       }
    } else {
       backgroundColor = theme->background.colorDisabled;
       textColor = theme->foreground.colorDisabled;
    }
    auto size = getSizeRect().embiggen(-THICKNESS);
    drawRectangleRounded(size, theme->outline.roundness, SEGMENTS, backgroundColor);
    drawRectangleRoundedLines(size, theme->outline.roundness, SEGMENTS, THICKNESS, COLORS::black);
    drawTextCentered(text, getSizeRect().center(), *theme->font, textColor, theme->font->size, theme->font->spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void ToggleButton::_on_down() {
   Button::_on_down();
   publish(ToggleButton::ButtonToggleEvent(this));
}

/////////////////////////////////////////////////////////////////////////////////////////
void ToggleButton::_on_up(bool mouseEscaped) {
   Button::_on_up(mouseEscaped);
   publish(ToggleButton::ButtonToggleEvent(this));
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void PushButton::_on_up(bool mouseEscaped) {
   Button::_on_up(mouseEscaped);
   publish(PushButton::ButtonPressEvent(this));
}

