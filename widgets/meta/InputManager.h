#pragma once
#include "Event.h"
#include "DrawInterface.h"

namespace EventType {
   DECLARE_EVENT(EVENT_INPUT)
   DECLARE_EVENT(EVENT_INPUT_BUTTON)
   DECLARE_EVENT(EVENT_INPUT_KEY)
   DECLARE_EVENT(EVENT_INPUT_MOUSE_BUTTON)
   DECLARE_EVENT(EVENT_INPUT_MOUSE_MOTION)
}

struct InputEvent : public Event {};
struct InputEventKey : public InputEvent{};
struct InputEventMouse : public InputEvent{};
struct InputEventMouseButton : public InputEventMouse{};
struct InputEventMouseMotion : public InputEventMouse{};

class InputManager
{
public:
   static InputManager& instance(){static InputManager instance; return instance;}
private:
   InputManager(){}
public:
   InputManager(InputManager const&) = delete;
   void operator=(InputManager const&) = delete;
   inline bool isKeyPressed(int key){IsKeyPressed(key);}
   inline bool isKeyDown(int key){return IsKeyDown(key);}
   inline bool isKeyReleased(int key){return IsKeyReleased(key);}
   inline bool isKeyUp(int key){return IsKeyUp(key);}
   inline void setExitKey(int key){return SetExitKey(key);}
};