#pragma once
#include "Event.h"
#include "DrawInterface.h"

struct InputEvent : public Event<BaseEvent> {
   EVENT_CTOR(InputEvent, Event<BaseEvent>, EVENT_INPUT);
};

struct InputEventKey : public InputEvent {
   EVENT_CTOR(InputEventKey, InputEvent, EVENT_INPUT_KEY)
   int key;
   bool isDown;
};

struct InputEventMouse : public InputEvent{
   EVENT_CTOR(InputEventMouse, InputEvent, EVENT_INPUT_MOUSE)
   GFCSDraw::Vec2<int> globalPos;
};

struct InputEventMouseButton : public InputEventMouse{
   EVENT_CTOR(InputEventMouseButton, InputEventMouse, EVENT_INPUT_MOUSE_BUTTON);
   InputInterface::MouseButton button;
   bool isDown;
};

struct InputEventMouseWheel : public InputEventMouse{
   EVENT_CTOR(InputEventMouseWheel, InputEventMouse, EVENT_INPUT_MOUSE_WHEEL);
   GFCSDraw::Vec2<int> wheelMove;
};

struct InputEventMouseMotion : public InputEventMouse{
   EVENT_CTOR(InputEventMouseMotion, InputEventMouse, EVENT_INPUT_MOUSE_MOTION);
   GFCSDraw::Vec2<int> mouseDelta;
};

enum class InputFilter {
   INPUT_FILTER_PASS_AND_PROCESS, //passes input to children and then process it locally if it's not handled by a child (default)
   INPUT_FILTER_PROCESS_AND_PASS, //processes input locally first, and passes it to children if we can't use it locally
   INPUT_FILTER_PROCESS_AND_STOP, //processes input locally but does not pass it to children
   INPUT_FILTER_IGNORE_AND_PASS, //passes input to children without handling it
   INPUT_FILTER_IGNORE_AND_STOP //ignores input and does not pass or handle it
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
   static inline bool isMouseButtonPressed(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonPressed(mouseButton);}
   static inline bool isMouseButtonDown(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonDown(mouseButton);}
   static inline bool isMouseButtonUp(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonUp(mouseButton);}
   static inline bool isMouseButtonReleased(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonReleased(mouseButton);}
   static inline bool isKeyPressed(InputInterface::KeyCode key){return InputInterface::isKeyPressed(key);}
   static inline bool isKeyDown(InputInterface::KeyCode key){return InputInterface::isKeyDown(key);}
   static inline bool isKeyReleased(InputInterface::KeyCode key){return InputInterface::isKeyReleased(key);}
   static inline bool isKeyUp(InputInterface::KeyCode key){return InputInterface::isKeyUp(key);}
   static inline void setExitKey(InputInterface::KeyCode key){return InputInterface::setExitKey(key);}
   static inline GFCSDraw::Vec2<int> getMousePos(){return InputInterface::getMousePos();}
   static inline GFCSDraw::Vec2<int> getMouseDelta(){return InputInterface::getMouseDelta();}
protected:
   InputInterface::KeyCode getKeyPressed();
   InputInterface::KeyCode getKeyReleased();
   InputInterface::MouseButton getMouseButtonPressed();
   InputInterface::MouseButton getMouseButtonReleased();

   std::vector<InputInterface::KeyCode> keyQueue; //holds keys that were pressed so we can check if they're still down
   std::vector<InputInterface::MouseButton> mouseButtonQueue; //holds mousebuttons that were pressed so we can check if they're still down
   template <typename T, typename R>
   bool isInQueue(std::vector<T> queue, R button){
     auto it = std::find(queue.begin(), queue.end(), button);
     return it != queue.end();
   }
   friend class Window;


};