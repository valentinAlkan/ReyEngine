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
      {}
      char ch = 0;
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

   namespace Internal{
      struct InputUnion {
         InputEventKey* key;
         InputEventChar* chr;
         InputEventMouse* mouse;
         InputEventMouseMotion* motion;
         InputEventMouseButton* button;
         InputEventMouseWheel* wheel;
         InputEventController* controller;
      };
   }

   //largest of the mouse input sizes
   struct InputEvent {
   private:
      void initCommon(bool _isMouse, EventId _eventId) {
         isMouse = _isMouse;
         eventId = _eventId;
      }
      template <typename T>
      void assign(T* ptr, const T& event) {
         ptr = &const_cast<T&>(event);
         eventId = T::ID;
         switch (T::ID) {
            case InputEventMouse::ID:
            case InputEventMouseMotion::ID:
            case InputEventMouseButton::ID:
            case InputEventMouseWheel::ID:
               isMouse = true;
               break;
            default:
               isMouse = false;
               break;
         }
      }
      Internal::InputUnion _union = {};
   public:
      InputEvent(const InputEventKey& event){assign<std::remove_cvref_t<decltype(event)>>(_union.key, event);}
      InputEvent(const InputEventChar& event){assign<std::remove_cvref_t<decltype(event)>>(_union.chr, event);}
      InputEvent(const InputEventMouseMotion& event){assign<std::remove_cvref_t<decltype(event)>>(_union.motion, event);}
      InputEvent(const InputEventMouseButton& event){assign<std::remove_cvref_t<decltype(event)>>(_union.button, event);}
      InputEvent(const InputEventMouseWheel& event){assign<std::remove_cvref_t<decltype(event)>>(_union.wheel, event);}
      InputEvent(const InputEventController& event){assign<std::remove_cvref_t<decltype(event)>>(_union.controller, event);}
      template <typename T>
      requires std::is_base_of_v<T, BaseEvent>
      constexpr const T& toEvent() {
         switch (T::ID) {
            case InputEventMouse::ID:{
               auto member = _union.mouse;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventMouseMotion::ID:{
               auto member = _union.motion;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventMouseButton::ID:{
               auto member = _union.button;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventMouseWheel::ID:{
               auto member = _union.wheel;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventKey::ID:{
               auto member = _union.key;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventChar::ID:{
               auto member = _union.chr;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
            case InputEventController::ID:{
               auto member = _union.controller;
               static_assert(std::is_same_v<decltype(member), T*>);
               return static_cast<const T&>(member);}
         }
      }
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