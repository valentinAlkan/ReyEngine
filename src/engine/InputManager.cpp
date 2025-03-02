#include "InputManager.h"

using namespace ReyEngine;
using namespace InputInterface;

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
char InputManager::getCharPressed() {
   return InputInterface::getCharPressed();
}

/////////////////////////////////////////////////////////////////////////////////////////
InputInterface::MouseButton InputManager::getMouseButtonPressed() {
   for (auto btn : MouseButtons) {
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