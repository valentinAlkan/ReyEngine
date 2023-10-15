#pragma once
#include "Event.h"
#include "DrawInterface.h"

namespace EventType {



}

struct InputEvent : public Event {
protected:
   InputEvent(): Event("InputEvent"){}
   explicit InputEvent(const std::string& eventType): Event(eventType){};
};

struct InputEventKey : public InputEvent {
   DECLARE_EVENT(EVENT_INPUT_KEY);
   InputEventKey(): InputEvent(EVENT_INPUT_KEY){}
   int key;
   bool isDown;
};

struct InputEventMouse : public InputEvent{
   GFCSDraw::Vec2<int> globalPos;
protected:
   explicit InputEventMouse(const std::string& eventType): InputEvent(eventType){};
};

struct InputEventMouseButton : public InputEventMouse{
   DECLARE_EVENT(EVENT_INPUT_MOUSE_BUTTON);
   InputEventMouseButton(): InputEventMouse(EVENT_INPUT_MOUSE_BUTTON){};
   InputInterface::MouseButton button;
   bool isDown;
};

struct InputEventMouseWheel : public InputEventMouse{
   DECLARE_EVENT(EVENT_INPUT_MOUSE_WHEEL)
   InputEventMouseWheel(): InputEventMouse(EVENT_INPUT_MOUSE_WHEEL){}
   GFCSDraw::Vec2<int> wheelMove;
};

struct InputEventMouseMotion : public InputEventMouse{
   DECLARE_EVENT(EVENT_INPUT_MOUSE_MOTION)
   InputEventMouseMotion(): InputEventMouse(EVENT_INPUT_MOUSE_MOTION){}
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
   inline bool isMouseButtonPressed(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonPressed(mouseButton);}
   inline bool isMouseButtonDown(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonDown(mouseButton);}
   inline bool isMouseButtonUp(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonUp(mouseButton);}
   inline bool isMouseButtonReleased(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonReleased(mouseButton);}
   inline bool isKeyPressed(InputInterface::KeyCode key){return InputInterface::isKeyPressed(key);}
   inline bool isKeyDown(InputInterface::KeyCode key){return InputInterface::isKeyDown(key);}
   inline bool isKeyReleased(InputInterface::KeyCode key){return InputInterface::isKeyReleased(key);}
   inline bool isKeyUp(InputInterface::KeyCode key){return InputInterface::isKeyUp(key);}
   inline void setExitKey(InputInterface::KeyCode key){return InputInterface::setExitKey(key);}
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