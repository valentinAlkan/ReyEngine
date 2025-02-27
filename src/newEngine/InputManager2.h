#pragma once
#include "Logger.h"
#include "Event2.h"
#include "ReyEngine.h"
#include <algorithm>

#define CONSTEXPR_EVENTID(EVENT_ID) static constexpr EventId ID = EVENT_ID;
#define EVENT(EVENT_NAME, UNIQUE_EVENT_ID) \
   struct EVENT_NAME : public Event<EVENT_NAME, UNIQUE_EVENT_ID> { \
      CONSTEXPR_EVENTID(UNIQUE_EVENT_ID)                   \
      explicit EVENT_NAME(const EventPublisher* publisher) : Event<EVENT_NAME, UNIQUE_EVENT_ID>(publisher)


#define EVENT_ARGS(EVENT_NAME, UNIQUE_EVENT_ID, ...) \
   struct EVENT_NAME : public Event<EVENT_NAME, UNIQUE_EVENT_ID> { \
      CONSTEXPR_EVENTID(UNIQUE_EVENT_ID)             \
      explicit EVENT_NAME(const EventPublisher* publisher, __VA_ARGS__) : Event<EVENT_NAME, UNIQUE_EVENT_ID>(publisher)

namespace ReyEngine{
   EVENT(InputEventKey, 13234646664){}
      InputInterface::KeyCode key;
      bool isDown = false;
      bool isRepeat = false;
   };

   EVENT(InputEventChar, 234623462346)
      , eventKey(publisher)
      {}
      char ch = 0;
      InputEventKey eventKey;
   };


   // mouse types
   EVENT_ARGS(InputEventMouse, 2346275324, const Pos<float>& pos)
      , canvasPos(pos)
      {}
      CanvasSpace<Pos<float>> canvasPos;
      Pos<float> localPos; //gets transformed
   };

   EVENT_ARGS(InputEventMouseButton, 85436723527, const Pos<float>& pos, InputInterface::MouseButton button, bool isDown)
      , mouse(publisher, pos)
      , button(button)
      , isDown(isDown)
      {}
      InputEventMouse mouse;
      InputInterface::MouseButton button;
      bool isDown;
   };

   EVENT_ARGS(InputEventMouseWheel, 386425732537, const Pos<int>& pos, const Vec2<float>& wheelMove)
      , wheelMove(wheelMove)
      , mouse(publisher, pos)
      {}
      InputEventMouse mouse;
      Vec2<float> wheelMove;
   };

   EVENT_ARGS(InputEventMouseMotion, 384765275842754, const Pos<int>& pos, const Vec2<float>& mouseDelta)
      , mouse(publisher, pos)
      , mouseDelta(mouseDelta)
      {}
      InputEventMouse mouse;
      Vec2<float> mouseDelta;
   };

   // other types
   EVENT(InputEventController, 134512435)
   {}
      /*nothing to see here*/
   };

   //largest of the mouse input sizes
   union InputEvent {
   private:
      void initCommon(bool _isMouse, EventId _eventId) {
         isMouse = _isMouse;
         eventId = _eventId;
      }
   public:
      InputEvent(const InputEventKey& eventKey): key(eventKey) {initCommon(false, InputEventKey::ID);}
      InputEvent(const InputEventChar& eventChar): chr(eventChar) {initCommon(false, InputEventChar::ID);}
      InputEvent(const InputEventMouseMotion& eventMotion): motion(eventMotion) {initCommon(true, InputEventMouseMotion::ID);}
      InputEvent(const InputEventMouseButton& eventButton): button(eventButton) {initCommon(true, InputEventMouseButton::ID);}
      InputEvent(const InputEventMouseWheel& eventWheel): wheel(eventWheel) {initCommon(true, InputEventMouseWheel::ID);}
      InputEvent(const InputEventController& controller): controller(controller) {initCommon(false, InputEventController::ID);}
      InputEventKey key;
      InputEventChar chr;
      InputEventMouse mouse;
      InputEventMouseMotion motion;
      InputEventMouseButton button;
      InputEventMouseWheel wheel;
      InputEventController controller;
      bool isMouse;
      EventId eventId;
   };

   enum class InputFilter {
      INPUT_FILTER_PASS_AND_PROCESS, //passes input to children and then process it locally if it's not handled by a child (default)
      INPUT_FILTER_PROCESS_AND_PASS, //processes input locally first, and passes it to children if we can't use it locally
      INPUT_FILTER_PROCESS_AND_STOP, //processes input locally but does not pass it to children
      INPUT_FILTER_IGNORE_AND_PASS, //passes input to children without handling it
      INPUT_FILTER_IGNORE_AND_STOP, //ignores input and does not pass or handle it
      INPUT_FILTER_PUBLISH_AND_PASS, //publishes subscribers input instead of handling it locally
      INPUT_FILTER_PASS_AND_PUBLISH, //publishes input subscribers instead of handling it locally
      INPUT_FILTER_PUBLISH_AND_STOP, //publishes input to subscribers instead of handling it locally
      INPUT_FILTER_PASS_PUBLISH_PROCESS, //
      INPUT_FILTER_PASS_PROCESS_PUBLISH, //
      INPUT_FILTER_PROCESS_PUBLISH_PASS, //
      INPUT_FILTER_PROCESS_PASS_PUBLISH, //
      INPUT_FILTER_PUBLISH_PASS_PROCESS, //
      INPUT_FILTER_PUBLISH_PROCESS_PASS, //
   };

   struct UnhandledMouseInput{
      bool isInside = false;
      Pos<float> localPos;
   };

   class InputManager2
   {
   public:
      static InputManager2& instance(){static InputManager2 instance; return instance;}
   private:
      InputManager2(){}
   public:
      InputManager2(InputManager2 const&) = delete;
      void operator=(InputManager2 const&) = delete;
      static inline bool isMouseButtonDown(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonDown(mouseButton);}
      static inline bool isMouseButtonUp(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonUp(mouseButton);}
      static inline bool isMouseButtonPressed(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonPressed(mouseButton);}
      static inline bool isMouseButtonReleased(InputInterface::MouseButton mouseButton){return InputInterface::isMouseButtonReleased(mouseButton);}
      static inline bool isKeyDown(InputInterface::KeyCode key){return InputInterface::isKeyDown(key);}
      static inline bool isKeyUp(InputInterface::KeyCode key){return InputInterface::isKeyUp(key);}
      static inline void setExitKey(InputInterface::KeyCode key){return InputInterface::setExitKey(key);}
      static inline bool isShiftKeyDown(){return InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_SHIFT) || InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_SHIFT);}
      static inline InputInterface::KeyCode getLastKeyPressed(){return instance()._lastKey;}
      static inline WindowSpace<Pos<float>> getMousePos(){return InputInterface::getMousePos();}
      static inline Vec2<float> getMouseDelta(){return InputInterface::getMouseDelta();}
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
      InputInterface::KeyCode _lastKey = InputInterface::KeyCode::KEY_NULL;
      std::vector<InputInterface::MouseButton> mouseButtonQueue; //holds mousebuttons that were pressed so we can check if they're still down
      template <typename T, typename R>
      bool isInQueue(std::vector<T> queue, R button){
         auto it = std::find(queue.begin(), queue.end(), button);
         return it != queue.end();
      }
      friend class Window2;
   };
}