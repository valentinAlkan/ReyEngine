#include "InputManager.h"
#include "Widget.h"

using namespace ReyEngine;
using namespace InputInterface;

///////////////////////////////////////////////////////////////////////////////////////////
Handled Internal::InputReceiver::__process_unhandled_input(const InputEvent& event) {
   //we can intercept certain events and always handle them, for example, tooltips (and eventually hovers)
   if (event.isMouse().has_value() && event.isMouse().value()->isInside()){
      switch (event.eventId) {
         //offer these events to children who may want them, but then accept them if there are no takers.
         case InputEventMouseToolTip::ID: {
            if (auto handled = _unhandled_input(event); !handled && !_tooltipText.empty())
               return {this, event.isMouse().value()->getLocalPos()};
            break;
         }
         case InputEventMouseHover::ID: {
            if (auto handled = _unhandled_input(event); !handled && _acceptsHover)
               return {this, event.isMouse().value()->getLocalPos()};
            break;
         }
      }
   }

   auto handler = _unhandled_input(event);
   if (event.isMouse() && handler.handler == this && !handler.pos.has_value()) {
      handler.pos = event.isMouse().value()->getLocalPos();
   }
   return handler;
}

/////////////////////////////////////////////////////////////////////////////////////////
KeyCode InputManager::getKeyPressed() {
   auto key = InputInterface::getKeyPressed();
   if ((int)key){
      keyQueue.push_back(key);
      _lastKey = key;
   }
   return key;
}

/////////////////////////////////////////////////////////////////////////////////////////
KeyCode InputManager::getKeyReleased() {
   for (auto it=keyQueue.begin(); it!=keyQueue.end(); it++){
      KeyCode key = *it;
      if (InputInterface::isKeyUp(key)){
         keyQueue.erase(it);
         if (keyQueue.empty()) {
            _lastKey = KeyCode::KEY_NULL;
         } else {
            _lastKey = keyQueue.front();
         }
         return key;
      }
   }
   return KeyCode::KEY_NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
int InputManager::getCharPressed() {
   return InputInterface::getCharPressed();
}

/////////////////////////////////////////////////////////////////////////////////////////
InputInterface::MouseButton InputManager::getMouseButtonPressed() {
   for (auto btn : MouseButtonsArray) {
      if (InputInterface::isMouseButtonDown(btn) && !isInQueue(mouseButtonQueue, btn)) {
         mouseButtonQueue.push_back(btn);
         return btn;
      }
   }
   return InputInterface::MouseButton::NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////
InputInterface::MouseButton InputManager::getMouseButtonReleased() {
   for (auto it=mouseButtonQueue.begin(); it!=mouseButtonQueue.end(); it++){
      InputInterface::MouseButton btn = *it;
      if (InputInterface::isMouseButtonUp(btn)){
         mouseButtonQueue.erase(it);
         return btn;
      }
   }
   return InputInterface::MouseButton::NONE;
}