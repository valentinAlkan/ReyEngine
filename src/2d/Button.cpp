#include "Button.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
Widget* Button::_unhandled_input(const InputEvent& event) {
    if (!enabled) return nullptr;
    if (event.isEvent<InputEventMouseButton>()) {
       auto mbEvent = event.toEvent<InputEventMouseButton>();
       if (mbEvent.button == InputInterface::MouseButton::LEFT) {
          bool isInside = event.isMouse().value()->isInside();
          if (down && !mbEvent.isDown) {
            //button is down and it was just released *somewhere*
             publish<ButtonPressEvent>(ButtonPressEvent(this));
            //if it was released on the button, it is a press
             publish<ButtonUpEvent>(ButtonUpEvent(this, isInside));
             down = false;
             return this;
          } else if (mbEvent.isDown && isInside) {
            //normal inside-click
             publish<ButtonDownEvent>(ButtonDownEvent(this));
             down = true;
             return this;
          }
       }
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::setDown(bool newDown, bool noEvents){
   down = newDown;
   if (!noEvents) {
      if (down) {
         publish<ButtonDownEvent>(ButtonDownEvent(this));
      } else {
         publish<ButtonUpEvent>(ButtonUpEvent(this, true));
         publish<ButtonPressEvent>(ButtonPressEvent(this));
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void PushButton::render2D() const {
    static constexpr int SEGMENTS = 10;
    static constexpr int THICKNESS = 1;
    ColorRGBA backgroundColor;
    ColorRGBA textColor;
    if (enabled) {
       backgroundColor = theme->background.colorPrimary;
       textColor = theme->font.color;
       if (isHovered()){backgroundColor = theme->background.colorSecondary;}
       if (down) backgroundColor = theme->background.colorTertiary;
    } else {
       backgroundColor = theme->background.colorDisabled;
       textColor = theme->foreground.colorDisabled;
    }
    drawRectangleRounded(getRect().toSizeRect(), theme->outline.roundness, SEGMENTS, backgroundColor);
    drawRectangleRoundedLines(getRect().toSizeRect().embiggen(-THICKNESS), theme->outline.roundness, SEGMENTS, THICKNESS, COLORS::black);
    drawTextCentered(text, getSizeRect().center(), theme->font, textColor, theme->font.size, theme->font.spacing);
}