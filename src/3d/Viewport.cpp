#include "Viewport.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::_init() {
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::renderChain() {
   //draw 3D first
   renderer3DChain();
   //draw 2D on top of 3D
   BaseWidget::renderChain();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::renderer3DBegin() {
   _activeCamera.get().push();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::renderer3DEnd(){
   _activeCamera.get().pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::render() const{
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::setUnhandledInputCallback(std::function<Handled(Viewport &, const InputEvent &, const std::optional<UnhandledMouseInput> &)> fx){
   unhandledInputCallback = fx;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Viewport::_unhandled_input(const InputEvent& inputEvent, const std::optional<UnhandledMouseInput>& mouseInput) {
   //offer up input to modal widget first
   if (unhandledInputCallback) {
      if (unhandledInputCallback(*this, inputEvent, mouseInput)) return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::_on_rect_changed() {
//   _defaultCamera.setTarget(Vec2<float>(getSize().x/2, getSize().y/2));
//   _defaultCamera.target = _defaultCamera.offset;
//   auto gpos = getGlobalPos();
//   _virtualInputOffset = Pos<int>(gpos.x, gpos.y);
}

///////////////////////////////////////////////////////////////////////////////////////
void Viewport::setActiveCamera(CameraTransform3D& newCamera) {
   _activeCamera = std::ref(newCamera);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::deleteActiveCamera() {
   _activeCamera = _defaultCamera;
}