#include "Viewport.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Viewport> Viewport::build(const std::string &instanceName) {
   auto viewport = shared_ptr<Viewport>(new Viewport(instanceName));
   return viewport;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::_init() {
}
/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::renderBegin(Pos<double>& textureOffset) {
   textureOffset -= _rect.value.pos();
   _activeCamera.get().push();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::render() const{
   if (_showGrid){
      DrawGrid(10, 1.0f);
      DrawCube({0,0,0}, 1,1,1, RED);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::renderEnd() {
   _activeCamera.get().pop();
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