#include "Button.h"
using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseButton::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
    if (_isEditorWidget) return false;
    if (!enabled) return false;
    switch (event.eventId) {
        case InputEventMouseButton::getUniqueEventId(): {
            auto mbEvent = event.toEventType<InputEventMouseButton>();
            if (mbEvent.button == InputInterface::MouseButton::LEFT) {
               if (down && !mbEvent.isDown){
                  //button is down and it was just released *somewhere*
                  auto toggle = ButtonToggleEvent(toEventPublisher(), mbEvent.isDown, mouse->isInside);
                  publish<ButtonToggleEvent>(toggle);
                  if (mouse->isInside){
                     //if it was released on the button, it is a press
                     auto press = ButtonPressEvent(toEventPublisher(), mbEvent.isDown);
                     publish<ButtonPressEvent>(press);
                  }
                  down = false;
                  return true;
               } else if (mbEvent.isDown && mouse->isInside) {
                  //normal inside-click
                  auto toggle = ButtonToggleEvent(toEventPublisher(), mbEvent.isDown, mouse->isInside);
                  publish<ButtonToggleEvent>(toggle);
                  down = true;
                  return true;
               }
               }
            }
        }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseButton::setDown(bool newDown){
   down = newDown;
   auto toggle = ButtonToggleEvent(toEventPublisher(), newDown, false);
   publish<ButtonToggleEvent>(toggle);
   auto press = ButtonPressEvent(toEventPublisher(), newDown);
   publish<ButtonPressEvent>(press);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void PushButton::render() const {
    static constexpr int SEGMENTS = 10;
    static constexpr int THICKNESS = 1;
    ColorRGBA backgroundColor;
    ColorRGBA textColor;
    if (enabled) {
       backgroundColor = theme->background.colorPrimary.value;
       textColor = theme->font.value.color;
       if (isHovered()) backgroundColor = theme->background.colorSecondary.value;
       if (down) backgroundColor = theme->background.colorTertiary.value;
    } else {
       backgroundColor = theme->background.colorDisabled;
       textColor = theme->foreground.colorDisabled;
    }
    drawRectangleRounded(getRect().toSizeRect(), theme->roundness.value, SEGMENTS, backgroundColor);
    drawRectangleRoundedLines(getRect().toSizeRect().embiggen(-THICKNESS), theme->roundness.value, SEGMENTS, THICKNESS, COLORS::black);
    drawTextCentered(text.value, getRect().toSizeRect().center(), theme->font.value, textColor, theme->font.value.size, theme->font.value.spacing);
}