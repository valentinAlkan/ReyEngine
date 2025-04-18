#include "Canvas.h"
#include "Application.h"
#include "rlgl.h"
#include "Camera2D.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_init() {
   if (!_renderTarget.ready()) {
      _renderTarget.setSize(Size<int>(getSize())); //todo: make protected
   }

   theme->background.colorPrimary.value = Colors::white;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DBegin() {
   //only clear owned render target.
   // otherwise we will assume that is being managed externally (by window, for instance)
   Application::getWindow(0).pushRenderTarget(_renderTarget);
   _renderTarget.clear();

   //apply the active camera transform
   auto camera = _activeCamera.lock();
   if (camera){
      rlPushMatrix();
      // Apply 2d camera transformation to modelview
      rlMultMatrixf(MatrixToFloat(camera->getCameraMatrix2D()));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DEnd() {
   //redraw the modal widget, if any
   auto modal = getModal();
   if (modal){
       auto modalWidget = modal.value().lock();
       if (modalWidget->_visible) modalWidget->render2DChain();
   }
   auto camera = _activeCamera.lock();
   if (camera){
      rlPopMatrix();
   }
   Application::getWindow(0).popRenderTarget();
   //draw what has been painted to this texture
   drawRenderTarget(_renderTarget, renderDst, theme->background.colorPrimary.value);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::__process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   //for mouse events, convert window coordinates to canvas space, then pass along the normal chain
//   if (mouse){
//      //the function we will use to transform input
//      auto xformFx = [this](const CanvasSpace<Pos<float>>& p){return screenToWorld(p);};
//      std::optional<UnhandledMouseInput> worldSpaceMouse = mouse;
//      worldSpaceMouse.value().localPos = xformFx(mouse->localPos); //is global/canvas space
//      worldSpaceMouse->isInside = isInside(worldSpaceMouse.value().localPos);
//      auto& mouseEvent = event.toEventType<InputEventMouse>();
//
//      InputEventMouseUnion _union(mouseEvent);
//      _union.mouse.globalPos = xformFx(_union.mouse.globalPos);
//
//      //offer up input to modal widget first
//      if (_modal){
//         auto widget = _modal.value().lock();
//         if (widget && widget->_visible){
//            auto modalMouseInput = mouse;
//            //translate to local for mouse input
//            if (modalMouseInput){
//               auto& mouseEvent = event.toEventType<InputEventMouse>();
//               modalMouseInput.value().localPos = widget->canvasToLocal(mouseEvent.globalPos);
//               modalMouseInput->isInside = widget->isInside(modalMouseInput->localPos);
//            }
//            //nobody else should get modal input
//            return widget->_process_unhandled_input(event, modalMouseInput);
//         }
//      }
//
//      return _process_unhandled_input(_union.mouse, worldSpaceMouse);
//   }
//   //non-mouse input
//
//   //offer to focused first
//   if (getFocus()){
//      auto focused = getFocus()->lock();
//      if (focused->_process_unhandled_input(event, {})) return true;
//   }
//   return _process_unhandled_input(event, {});
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::_unhandled_input(const InputEvent& inputEvent, const std::optional<UnhandledMouseInput>& mouseInput) {
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
void Canvas::setFocus(std::shared_ptr<BaseWidget>& newFocus){
   if (_focus){
      auto focus = _focus->lock();
      if (focus){
         focus->_isFocus = false;
         focus->_on_focus_lost();
      }
   }
   _focus = newFocus;
   newFocus->_isFocus = true;
   newFocus->_on_focus_gained();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::clearFocus() {
   if (_focus){
      auto focus = _focus->lock();
      if (focus){
         focus->_isFocus = false;
         focus->_on_focus_lost();
      }
   }
   _focus.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   _renderTarget.setSize(getSize());
   setInputOffset(getPos());
}


/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::pushScissor(const CanvasSpace<ReyEngine::Rect<R_FLOAT>>& newArea) {
   if (!_scissorStack.empty()){
      _scissorStack.push(_scissorStack.top().getOverlap(newArea.get()));
   } else {
      _scissorStack.push(newArea.get());
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
Pos<R_FLOAT> Canvas::screenToWorld(const CanvasSpace<Pos<float>>& pos) const {
   auto camera = _activeCamera.lock();
   if (!camera) return pos.get();
   return camera->screenToWorld(pos.get());
}

///////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<R_FLOAT>> Canvas::worldToScreen(const Pos<R_FLOAT>& pos) const {
   auto camera = _activeCamera.lock();
   if (!camera) return pos;
   return camera->worldToscreen(pos);
}