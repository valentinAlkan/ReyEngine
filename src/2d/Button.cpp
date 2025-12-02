#include "Button.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Button::_unhandled_input(const InputEvent& event) {
   if (!enabled) return nullptr;
   if (event.isEvent<InputEventMouseButton>()) {
      auto& mbEvent = event.toEvent<InputEventMouseButton>();
      if (mbEvent.button == InputInterface::MouseButton::LEFT) {
         bool isInside = event.isMouse().value()->isInside();
         bool isPress = mbEvent.isDown && isInside;
         bool isRelease = isFocused() && !mbEvent.isDown;
         if (isPress && isInside) {
            //normal inside-click
            _drawState = DrawState::DOWN_PRESS;
            setFocused(true);
            return this;
         } else if (isRelease){
            //mouse release
            if (!_isToggle || _down){
               _down = false;
               _drawState = DrawState::UP;
               _on_up(isInside);
            } else if (!_down){
               _down = true;
               _on_down();
               _drawState = DrawState::DOWN;
            }
            setFocused(false);
            return this;
         }
      }
   }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Button::setDown(bool newDown, PublishType pubTybe){
   if (pubTybe == PublishType::DO_PUBLISH) {
      if (newDown && !getDown()) {
         _down = newDown;
         _drawState = DrawState::DOWN;
         _on_down();
      } else if (!newDown && getDown()){
         _down = newDown;
         _drawState = DrawState::UP;
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
       if (_drawState == DrawState::DOWN) {
          backgroundColor = theme->background.colorActive1;
       } else if (_drawState == DrawState::DOWN_PRESS) {
          backgroundColor = theme->background.colorActive2;
       }
       if (isHovered()){
          backgroundColor = theme->background.colorHighlight;
       }
    } else {
       backgroundColor = theme->background.colorDisabled;
       textColor = theme->foreground.colorDisabled;
    }
    auto size = getSizeRect().embiggen(-THICKNESS);
    drawRectangleRounded(size, theme->outline.roundness, SEGMENTS, backgroundColor);
    drawRectangleRoundedLines(size, theme->outline.roundness, SEGMENTS, THICKNESS, COLORS::black);
    if (_drawState == DrawState::DOWN) {
       drawRectangleRoundedLines(size.embiggen(-1), theme->outline.roundness, SEGMENTS, 1.0, COLORS::lightGray);
    } else if (_drawState == DrawState::DOWN_PRESS){
       drawRectangleRoundedLines(size.embiggen(-1), theme->outline.roundness, SEGMENTS, 1.0, COLORS::gray);
    } else {
       drawRectangleRoundedLines(size.embiggen(-1), theme->outline.roundness, SEGMENTS, 1.0, COLORS::white);
    }
    drawTextCentered(text, getSizeRect().center(), *theme->font, textColor, theme->font->size, theme->font->spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Button::getDown() const {
   return _drawState == DrawState::DOWN || _drawState == DrawState::DOWN_PRESS;

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
   if (isHovered()) {
      drawRectangleLines(_outline, 2.0, theme->background.colorHighlight);
   }
   drawRectangleLines(_box, 1.0, theme->foreground.colorSecondary);
//   drawRectangleLines(_textRect, 1.0, theme->foreground.colorTertiary);
   if (getDown()) {
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