#include "Button.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Handled Button::_unhandled_input(const InputEvent& event) {
    if (!enabled) return false;
    if (event.isEvent<InputEventMouseButton>()) {
       auto mbEvent = event.toEvent<InputEventMouseButton>();
       if (mbEvent.button == InputInterface::MouseButton::LEFT) {
          bool isInside = event.isMouse().value()->isInside();
          if (down && !mbEvent.isDown) {
            //button is down and it was just released *somewhere*
             auto toggle = ButtonToggleEvent(this, mbEvent.isDown, isInside);
             publish<ButtonToggleEvent>(toggle);
             if (isInside) {
               //if it was released on the button, it is a press
                auto press = ButtonPressEvent(this, mbEvent.isDown);
                publish<ButtonPressEvent>(press);
             }
             down = false;
             return true;
          } else if (mbEvent.isDown && isInside) {
            //normal inside-click
             auto toggle = ButtonToggleEvent(this, mbEvent.isDown, isInside);
             publish<ButtonToggleEvent>(toggle);
             down = true;
             return true;
          }
       }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::setDown(bool newDown, bool noEvents){
   down = newDown;
   auto toggle = ButtonToggleEvent(this, newDown, false);
   publish<ButtonToggleEvent>(toggle);
   auto press = ButtonPressEvent(this, newDown);
   publish<ButtonPressEvent>(press);
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
    drawTextCentered(text, getRect().toSizeRect().center(), theme->font, textColor, theme->font.size, theme->font.spacing);
}