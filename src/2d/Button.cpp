#include "Button.h"
/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseButton::_unhandled_input(InputEvent& event, std::optional<UnhandledMouseInput> mouse) {
    if (_isEditorWidget) return false;
    switch (event.eventId) {
        case InputEventMouseButton::getUniqueEventId(): {
            auto mouseEvent = event.toEventType<InputEventMouseButton>();
            if (mouseEvent.button == InputInterface::MouseButton::MOUSE_BUTTON_LEFT) {
               if (mouse->isInside && mouseEvent.isDown) {
                  setDown(true);
                  return true;
               } else if (!mouseEvent.isDown){
                  setDown(false);
               }
            }
        }
        break;
        case InputEventMouseMotion::getUniqueEventId():
            auto mouseEvent = event.toEventType<InputEventMouseMotion>();
            /**/
            break;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseButton::setDown(bool newDown){
    if (wasDown != newDown){
        down.set(newDown);
        auto e = ButtonPressEvent(toEventPublisher(), newDown);
        publish<ButtonPressEvent>(e);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void PushButton::render() const {
    static constexpr int SEGMENTS = 10;
    static constexpr int THICKNESS = 2;
    auto color = theme->background.colorPrimary.value;
    if (isHovered()) color = theme->background.colorSecondary.value;
    if (down.value) color = theme->background.colorTertiary.value;
    _drawRectangleRounded(_rect.value.toSizeRect(), theme->roundness.value, SEGMENTS, color);
    _drawRectangleRoundedLines(_rect.value.toSizeRect(), theme->roundness.value, SEGMENTS, THICKNESS, COLORS::black);
    _drawTextCentered(text.value, _rect.value.toSizeRect().center(), theme->font.value);
}