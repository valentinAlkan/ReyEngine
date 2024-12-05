#include "Canvas.h"
#include "Application.h"
#include "rlgl.h"
#include "Camera2D.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::_init() {
   if (!_renderTarget.ready()) {
      _renderTarget.setSize(Size<int>(getSize())); //todo: make protected
   }

   // the canvas' own transform acts as the default camera
//   _cameraStack.push(&_transform.value);
}
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderBegin(ReyEngine::Pos<R_FLOAT>& textureOffset) {
   //only clear owned render target.
   // otherwise we will assume that is being managed externally (by window, for instance)
   if (_renderTargetPtr) {
      Application::getWindow(0).pushRenderTarget(_renderTarget);
      _renderTarget.clear();
   }
   textureOffset -= getPos();
   //apply the active camera transform
   auto camera = _activeCamera.lock();
   if (camera){
      rlPushMatrix();
      // Apply 2d camera transformation to modelview
      rlMultMatrixf(MatrixToFloat(camera->getCameraMatrix2D()));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::render() const{
//   drawRectangleLines(getRect().toSizeRect(), 2.0, Colors::red);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderEnd() {
   //redraw the modal widget, if any
   auto modal = getModal();
   if (modal){
       auto modalWidget = modal.value().lock();
       Pos<R_FLOAT> toffset;
       if (modalWidget->_visible) modalWidget->renderChain(toffset);
   }
   auto camera = _activeCamera.lock();
   if (camera){
      rlPopMatrix();
   }
   if (_renderTargetPtr) {
      Application::getWindow(0).popRenderTarget();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::__process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   //for mouse events, convert global coordinates to world space, then pass along the normal chain
   std::optional<UnhandledMouseInput> worldSpaceMouse = mouse;
   if (mouse){
      worldSpaceMouse.value().localPos = screenToWorld(mouse->localPos);
      worldSpaceMouse->isInside = isInside(worldSpaceMouse.value().localPos);
   }

   switch (event.eventId){
      case InputEventMouseMotion::getUniqueEventId():
      case InputEventMouseButton::getUniqueEventId():
      case InputEventMouseWheel::getUniqueEventId():{
         //have to make sure we store enough memory to copy correctly - we won't know the size in advance
         union InputEventUnion {
            InputEventMouseMotion motion;
            InputEventMouseButton button;
            InputEventMouseWheel wheel;
         };
         char raw[sizeof(InputEventUnion)];
         //just go ahead and copy off the end, we don't really care what's there
         memcpy(raw, &event, sizeof(InputEventUnion));
         const auto& _event = event.toEventType<InputEventMouse>();
         auto& _worldSpaceEvent = reinterpret_cast<InputEventMouse&>(raw);
         _worldSpaceEvent.globalPos = screenToWorld(_event.globalPos);
         return _process_unhandled_input(reinterpret_cast<InputEvent&>(raw), worldSpaceMouse);
         }
   }
   return _process_unhandled_input(event, mouse);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ReyEngine::Canvas::_unhandled_input(const InputEvent& inputEvent, const std::optional<UnhandledMouseInput>& mouseInput) {
   //offer up input to modal widget first
   if (_modal){
      auto widget = _modal.value().lock();
      if (widget && widget->_visible){
         auto modalMouseInput = mouseInput;
         //translate to local for mouse input
         if (modalMouseInput){
            auto& mouseEvent = inputEvent.toEventType<InputEventMouse>();
            modalMouseInput.value().localPos = Pos<int>(widget->globalToLocal(mouseEvent.globalPos));
            modalMouseInput->isInside = widget->isInside(modalMouseInput->localPos);
         }
         if (widget->_process_unhandled_input(inputEvent, modalMouseInput)){
            return true;
         }
      }
   }

   if (unhandledInputCallback) {
       if (unhandledInputCallback(*this, inputEvent, mouseInput)) return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::setModal(std::shared_ptr<BaseWidget>& newModal) {
   if (_modal){
      auto modal = _modal->lock();
      if (modal){
         modal->_isModal = false;
         modal->_on_modality_lost();
      }
   }
   _modal = newModal;
   newModal->_isModal = true;
   newModal->_on_modality_gained();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::clearModal() {
   if (_modal){
      auto modal = _modal->lock();
      if (modal){
         modal->_isModal = false;
         modal->_on_modality_lost();
      }
   }
   _modal.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   //only resize the render target if we are the owner
   if (_renderTargetPtr) {
      _renderTarget.setSize(getSize());
   }
//   _defaultCamera.setTarget(Vec2<float>(getSize().x/2, getSize().y/2));
//   _defaultCamera.target = _defaultCamera.offset;
//   auto gpos = getGlobalPos();
//   _virtualInputOffset = Pos<R_FLOAT>(gpos.x, gpos.y);
}


/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::pushScissor(const ReyEngine::Rect<R_FLOAT>& newArea) {
   bool hastop = !_scissorStack.empty();
   if (hastop) {
      _scissorStack.push(_scissorStack.top().getOverlap(newArea));
   } else {
      _scissorStack.push(newArea);
   }
   auto& area = _scissorStack.top();
   BeginScissorMode(area.x, area.y, area.width, area.height);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::popScissor() {
   _scissorStack.pop();
   if (!_scissorStack.empty()) {
      auto& area = _scissorStack.top();
      BeginScissorMode(area.x, area.y, area.width, area.height);
   } else {
      EndScissorMode();
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Canvas::setActiveCamera(std::shared_ptr<ReyEngine::Camera2D>& camera) {
   _activeCamera = camera;
}

///////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> Canvas::screenToWorld(const Pos<R_FLOAT>& pos) const {
   auto camera = _activeCamera.lock();
   if (!camera) return pos;
   return camera->screenToWorld(pos);
}

///////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> Canvas::worldToScreen(const Pos<R_FLOAT>& pos) const {
   auto camera = _activeCamera.lock();
   if (!camera) return pos;
   return camera->worldToscreen(pos);
}