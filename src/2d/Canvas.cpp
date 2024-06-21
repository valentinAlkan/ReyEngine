#include "Canvas.h"
#include "Application.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::_init() {
   if (!_renderTarget.ready()) {
      _renderTarget.setSize(getSize()); //todo: make protected
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderBegin(ReyEngine::Pos<double>& textureOffset) {
   Application::instance().getWindow(0)->pushRenderTarget(_renderTarget);
   _renderTarget.clear();
   textureOffset -= _rect.value.pos();
   _activeCamera.get().push();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::render() const{
   drawRectangleLines(_rect.value.toSizeRect(), 2.0, Colors::red);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderEnd() {
   //redraw the modal widget, if any
   auto modal = getModal();
   if (modal){
       auto modalWidget = modal.value().lock();
       Pos<double> toffset;
       if (modalWidget->_visible) modalWidget->renderChain(toffset);
   }
  _activeCamera.get().pop();
   Application::instance().getWindow(0)->popRenderTarget();
   drawRenderTargetRect(_renderTarget, Rect<int>(_renderTarget.getSize()), {0,0});
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
            modalMouseInput.value().localPos = widget->globalToLocal(mouseEvent.globalPos);
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
   _renderTarget.setSize(_rect.value.size());
//   _defaultCamera.setTarget(Vec2<float>(getSize().x/2, getSize().y/2));
//   _defaultCamera.target = _defaultCamera.offset;
//   auto gpos = getGlobalPos();
//   _virtualInputOffset = Pos<int>(gpos.x, gpos.y);
}


/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::pushScissor(const ReyEngine::Rect<int>& newArea) {
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
void Canvas::setActiveCamera(CameraTransform& newCamera) {
   _activeCamera = std::ref(newCamera);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::deleteActiveCamera() {
   _activeCamera = _defaultCamera;
}