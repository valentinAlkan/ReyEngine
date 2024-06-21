#include "Viewport.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Viewport::_init() {
   if (!_renderTarget.ready()) {
      _renderTarget.setSize(getSize());
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Viewport::renderBegin(ReyEngine::Pos<double>& textureOffset) {
   Application::instance().getWindow(0)->pushRenderTarget(_renderTarget);
   _renderTarget.clear();
   textureOffset -= _rect.value.pos();
   _activeCamera.get().push();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Viewport::render() const{
   drawRectangleLines(_rect.value.toSizeRect(), 2.0, Colors::blue);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Viewport::renderEnd() {
   _activeCamera.get().pop();
   Application::instance().getWindow(0)->popRenderTarget();
   drawRenderTargetRect(_renderTarget, Rect<int>(_renderTarget.getSize()), {0,0});
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ReyEngine::Viewport::_unhandled_input(const InputEvent& inputEvent, const std::optional<UnhandledMouseInput>& mouseInput) {
   //offer up input to modal widget first
   if (unhandledInputCallback) {
      if (unhandledInputCallback(*this, inputEvent, mouseInput)) return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Viewport::_on_rect_changed() {
   _renderTarget.setSize(_rect.value.size());
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