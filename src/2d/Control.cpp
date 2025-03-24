#include "Control.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void Control::render2DBegin() {
   if (_doScissor){
//      startScissor(scissorArea);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Control::render2D() const {
   if (renderCallback) {
      renderCallback(*this);
   } else {
      drawRectangle(Internal::Drawable2D::getRect().toSizeRect(), Colors::red);
//      switch(theme->background.value){
//         case Style::Fill::SOLID:
//            drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary.value);
//            break;
//         case Style::Fill::GRADIENT:
//            drawRectangleGradientV(getRect().toSizeRect(), theme->background.colorPrimary.value, theme->background.colorSecondary.value);
//            break;
//         default:
//            break;
//      }
//      switch(theme->outline.value){
//         case Style::Outline::LINE:
//            drawRectangleLines(getRect().toSizeRect(), theme->outline.thickness, theme->outline.color);
//            break;
//         default:
//            break;
//      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Control::render2DEnd() {
   if (_doScissor){
//      stopScissor();
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void Control::_process(float dt) {
   if (processCallback) processCallback(*this, dt);
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Control::_unhandled_input(const InputEvent& event) {
   if (unhandledInputCallback) return unhandledInputCallback(*this, event);
   return nullptr;
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