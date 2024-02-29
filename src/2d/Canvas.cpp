#include "Canvas.h"

using namespace std;
using namespace ReyEngine;
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderBegin(ReyEngine::Pos<double>& textureOffset) {
   target.beginRenderMode();
   ClearBackground(ReyEngine::Colors::none);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Canvas::renderEnd() {
   //redraw the modal widget, if any
   auto modal = getModal();
   if (modal){
      modal.value().lock()->render();
   }
   target.endRenderMode();
   target.render(getGlobalRect().pos());
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ReyEngine::Canvas::_unhandled_input(const InputEvent& inputEvent, const std::optional<UnhandledMouseInput>& mouseInput) {
   //offer up input to modal widget first
   if (_modal){
      auto widget = _modal.value().lock();
      if (widget){
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