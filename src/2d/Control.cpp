#include "Control.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void Control::renderBegin(ReyEngine::Pos<double> &textureOffset) {
   if (_doScissor){
      startScissor(scissorArea);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Control::render() const {
   if (renderCallback) {
      renderCallback(*this);
   } else {
      switch(theme->background.value){
         case Style::Fill::SOLID:
            drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary.value);
            break;
         case Style::Fill::GRADIENT:
            drawRectangleGradientV(getRect().toSizeRect(), theme->background.colorPrimary.value, theme->background.colorSecondary.value);
            break;
         default:
            break;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Control::renderEnd() {
   if (_doScissor){
      stopScissor();
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

/////////////////////////////////////////////////////////////////////////////////////////
void Control::setScissorArea(const ReyEngine::Rect<int>& area) {
   _doScissor = true;
   scissorArea = area;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Control::clearScissorArea() {
   _doScissor = false;
}