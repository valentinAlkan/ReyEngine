#include "Control.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
void Control::render() const {
   if (renderCallback) {
      renderCallback();
   } else {
      if (theme->background.value == Style::Fill::SOLID) {
         _drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary.value);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_process(float dt) {
   if (processCallback) processCallback(dt);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Control::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   if (unhandledInputCallback) return unhandledInputCallback(event, mouse);
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_mouse_enter() {
   if (mouseEnterCallback) mouseEnterCallback();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_mouse_exit() {
   if (mouseExitCallback) mouseExitCallback();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_rect_changed() {
   if (rectChangedCallback) rectChangedCallback();
}