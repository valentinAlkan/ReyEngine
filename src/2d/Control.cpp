#include "Control.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
void Control::render() const {
   if (renderCallback) {
      renderCallback(*this);
   } else {
      if (theme->background.value == Style::Fill::SOLID) {
         drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary.value);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_process(float dt) {
   if (processCallback) processCallback(*this, dt);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Control::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   if (unhandledInputCallback) return unhandledInputCallback(*this, event, mouse);
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_mouse_enter() {
   if (mouseEnterCallback) mouseEnterCallback(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_mouse_exit() {
   if (mouseExitCallback) mouseExitCallback(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Control::_on_rect_changed() {
   if (rectChangedCallback) rectChangedCallback(*this);
}