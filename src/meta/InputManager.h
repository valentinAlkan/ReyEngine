#pragma once
#include "Event.h"
#include "DrawInterface.h"
#include <algorithm>

namespace ReyEngine{
   struct InputEvent : public ReyEngine::Event<InputEvent> {
      EVENT_CTOR_SIMPLE_OVERRIDABLE(InputEvent, Event<InputEvent>){}
   };

   struct InputEventKey : public InputEvent {
      EVENT_CTOR_SIMPLE(InputEventKey, InputEvent){}
      InputInterface::KeyCode key;
      bool isDown;
      bool isRepeat;
   };

   struct InputEventChar : public InputEvent {
      EVENT_CTOR_SIMPLE(InputEventChar, InputEvent){}
      char ch;
   };


   struct InputEventMouse : public InputEvent{
      EVENT_CTOR_SIMPLE_OVERRIDABLE(InputEventMouse, InputEvent){}
      ReyEngine::Pos<int> globalPos;
   };

   struct InputEventMouseButton : public InputEventMouse{
      EVENT_CTOR_SIMPLE(InputEventMouseButton, InputEventMouse){}
      InputInterface::MouseButton button;
      bool isDown;
   };

   struct InputEventMouseWheel : public InputEventMouse{
      EVENT_CTOR_SIMPLE(InputEventMouseWheel, InputEventMouse){}
      ReyEngine::Vec2<int> wheelMove;
   };

   struct InputEventMouseMotion : public InputEventMouse{
      EVENT_CTOR_SIMPLE(InputEventMouseMotion, InputEventMouse){}
      ReyEngine::Vec2<int> mouseDelta;
   };

   enum class InputFilter {
      INPUT_FILTER_PASS_AND_PROCESS, //passes input to children and then process it locally if it's not handled by a child (default)
      INPUT_FILTER_PROCESS_AND_PASS, //processes input locally first, and passes it to children if we can't use it locally
      INPUT_FILTER_PROCESS_AND_STOP, //processes input locally but does not pass it to children
      INPUT_FILTER_IGNORE_AND_PASS, //passes input to children without handling it
      INPUT_FILTER_IGNORE_AND_STOP //ignores input and does not pass or handle it
   };

   struct UnhandledMouseInput{
      bool isInside = false;
      ReyEngine::Pos<int> localPos;
   };

   class InputManager
   {
   public:
      static InputManager& instance(){static InputManager instance; return instance;}
   private:
      InputManager(){}
   public:
      InputManager(InputManager const&) = delete;
      void operator=(InputManager const&) = delete;
      static inline bool isMouseButtonDown(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonDown(mouseButton);}
      static inline bool isMouseButtonUp(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonUp(mouseButton);}
      static inline bool isMouseButtonPressed(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonPressed(mouseButton);}
      static inline bool isMouseButtonReleased(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonReleased(mouseButton);}
      static inline bool isKeyDown(InputInterface::KeyCode key){return InputInterface::isKeyDown(key);}
      static inline bool isKeyUp(InputInterface::KeyCode key){return InputInterface::isKeyUp(key);}
      static inline void setExitKey(InputInterface::KeyCode key){return InputInterface::setExitKey(key);}
      static inline bool isShiftKeyDown(){return InputInterface::isKeyDown(InputInterface::KeyCodes::KEY_LEFT_SHIFT) || InputInterface::isKeyDown(InputInterface::KeyCodes::KEY_RIGHT_SHIFT);}
      static inline InputInterface::KeyCode getLastKeyPressed(){return instance()._lastKey;}
      static inline ReyEngine::Pos<int> getMousePos(){return InputInterface::getMousePos();}
      static inline ReyEngine::Vec2<int> getMouseDelta(){return InputInterface::getMouseDelta();}
   protected:
      static inline bool isKeyPressed(InputInterface::KeyCode key){return InputInterface::isKeyPressed(key);}
      static inline bool isKeyReleased(InputInterface::KeyCode key){return InputInterface::isKeyReleased(key);}
      InputInterface::KeyCode getKeyPressed();
      InputInterface::KeyCode getKeyReleased();
      char getCharPressed();
      InputInterface::MouseButton getMouseButtonPressed();
      InputInterface::MouseButton getMouseButtonReleased();
      static inline Vec2<double> getMouseWheel(){return InputInterface::getMouseWheel();}

      std::vector<InputInterface::KeyCode> keyQueue; //holds keys that were pressed so we can check if they're still down
      InputInterface::KeyCode _lastKey = InputInterface::KeyCodes::KEY_NULL;
      std::vector<InputInterface::MouseButton> mouseButtonQueue; //holds mousebuttons that were pressed so we can check if they're still down
      template <typename T, typename R>
      bool isInQueue(std::vector<T> queue, R button){
        auto it = std::find(queue.begin(), queue.end(), button);
        return it != queue.end();
      }
      friend class Window;
   };
}