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
            _toggleState = !_toggleState;
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
          backgroundColor = theme->background.colorActive;
       } else if (isHovered()){
          backgroundColor = theme->background.colorHighlight;
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

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void CheckBox::render2D() const {
//   drawRectangleLines(_outline, 1.0, theme->outline.colorPrimary);
   drawRectangleLines(_box, 1.0, theme->foreground.colorSecondary);
//   drawRectangleLines(_textRect, 1.0, theme->foreground.colorTertiary);
   if (_toggleState) {
      drawLine(_box.backSlash(), 1.0, theme->foreground.colorPrimary);
      drawLine(_box.frontSlash(), 1.0, theme->foreground.colorPrimary);
   }
   drawText(text, _textRect.pos(), theme->font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CheckBox::_on_rect_changed() {
   _outline = getSizeRect().embiggen(-4);
   //recalculate box size
   _box = _outline.copy().embiggen(-14).left().midpoint().toCenterRect({15, 15});
   auto textRectPosX = _box.topRight().pushX(5).x;
   auto textRectPosY = _outline.topLeft().pushY(4).y;
   auto textRectWidth = _outline.right().a.x - textRectPosX - _outline.x;
   auto textRectHeight = (float)_outline.right().shorten(4).distance() - textRectPosY;
   _textRect = {{textRectPosX, textRectPosY}, {textRectWidth, textRectHeight}};
}