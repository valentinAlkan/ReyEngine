#include "InputManager.h"

using namespace InputInterface;

/////////////////////////////////////////////////////////////////////////////////////////
KeyCode InputManager::getKeyPressed() {
   auto key = InputInterface::getKeyPressed();
   if (key){
      keyQueue.push_back(key);
   }
   return key;
}

/////////////////////////////////////////////////////////////////////////////////////////
KeyCode InputManager::getKeyReleased() {

   for (auto it=keyQueue.begin(); it!=keyQueue.end(); it++){
      KeyCode key = *it;
      if (InputInterface::isKeyUp(key)){
         keyQueue.erase(it);
         return key;
      }
   }
   return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
InputInterface::MouseButton InputManager::getMouseButtonPressed() {
   for (auto btn : MouseButtons) {
      if (InputInterface::isMouseButtonDown(btn) && !isInQueue(mouseButtonQueue, btn)) {
         mouseButtonQueue.push_back(btn);
         return btn;
      }
   }
   return InputInterface::MouseButton::MOUSE_BUTTON_NONE;
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
   return InputInterface::MouseButton::MOUSE_BUTTON_NONE;
}