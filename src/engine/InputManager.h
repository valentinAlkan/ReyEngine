#pragma once
#include "Logger.h"
#include "Event.h"
#include "ReyEngine.h"
#include <algorithm>

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
   struct MouseEvent {
      explicit MouseEvent(const CanvasSpace<Pos<float>>& canvasPos)
      : _canvasPos(canvasPos)
      , _localPos(canvasPos.get())
      {}
      [[nodiscard]] CanvasSpace<Pos<float>> getCanvasPos() const {return _canvasPos;}
      [[nodiscard]] Pos<float> getLocalPos() const {return _localPos;}
      [[nodiscard]] bool isInside() const {return _isInside;}
   protected:
      void transformLocalPos(const Transform2D& xform) {
         //account for other transformations?
         _localPos = Pos<float>(xform.inverse().transform(_localPos));
      }
      void transformLocalPos(const Pos<float>& newPos){_localPos = newPos;}
   public:
      //friend access
      struct ScopeTransformer {
         ScopeTransformer(MouseEvent& mouseEvent, const Transform2D& xform, const Size<float>& localSize, std::optional<Transform2D> cameraTransform = {})
         : mouseEvent(mouseEvent)
         , cameraTransform(cameraTransform)
         , _isinside_cached(mouseEvent._isInside)
         , _localPos_cached(mouseEvent._localPos)
         {
            if (cameraTransform) {
               mouseEvent.transformLocalPos(cameraTransform.value());
            }
            mouseEvent.transformLocalPos(xform);
            mouseEvent._isInside = localSize.toRect().contains(getLocalPos());
         }
         ~ScopeTransformer(){
            mouseEvent.transformLocalPos(_localPos_cached);
            mouseEvent._isInside = _isinside_cached;
         }
         [[nodiscard]] Pos<float> getLocalPos() const {return mouseEvent._localPos;}
         [[nodiscard]] CanvasSpace<Pos<float>> getCanvasPos() const {return mouseEvent._canvasPos;}
      private:
         MouseEvent& mouseEvent;
         std::optional<Transform2D> cameraTransform;
         bool _isinside_cached; //for recall on dtor
         Pos<float> _localPos_cached; //for recall on dtor
      };
   private:
      CanvasSpace<Pos<float>> _canvasPos;
      Pos<float> _localPos;
      bool _isInside = false;
   };

   EVENT_ARGS(InputEventMouseButton, 111111, const Pos<float>& pos, InputInterface::MouseButton button, bool isDown)
      , mouse(pos)
      , button(button)
      , isDown(isDown)
      {}
      MouseEvent mouse;
      InputInterface::MouseButton button;
      bool isDown;
   };

   EVENT_ARGS(InputEventMouseWheel, 22222222, const Pos<float>& pos, const Vec2<float>& wheelMove)
      , mouse(pos)
      , wheelMove(wheelMove)
      {}
      MouseEvent mouse;
      Vec2<float> wheelMove;
   };

   EVENT_ARGS(InputEventMouseMotion, 333333333, const Pos<float>& pos, const Vec2<float>& mouseDelta)
      , mouse(pos)
      , mouseDelta(mouseDelta)
      {}
      MouseEvent mouse;
      Vec2<float> mouseDelta;
   };

   EVENT_ARGS(InputEventMouseHover, 3434343434, const Pos<float>& pos)
      , mouse(pos)
      {}
      MouseEvent mouse;
   };

   // other types
   EVENT(InputEventController, 4444444444444)
   {}
      /*nothing to see here*/
   };

   namespace Internal{
      struct InputUnion {
         BaseEvent* base;
         InputEventKey* key;
         InputEventChar* chr;
         InputEventMouseMotion* motion;
         InputEventMouseHover* hover;
         InputEventMouseButton* button;
         InputEventMouseWheel* wheel;
         InputEventController* controller;
      };
   }

   //largest of the mouse input sizes
   struct InputEvent {
   private:
      template <typename T>
      void assign(T*& unionMember, T& event) {
         //assign the even to the correct union member
         unionMember = &const_cast<T&>(event);
         eventId = T::ID;
         if constexpr (T::ID == InputEventMouseMotion::ID){
            _mouseData = &_union.motion->mouse;
         }
         if constexpr (T::ID == InputEventMouseHover::ID){
            _mouseData = &_union.hover->mouse;
         }
         if constexpr (T::ID == InputEventMouseButton::ID){
            _mouseData = &_union.button->mouse;
         }
         if constexpr (T::ID == InputEventMouseWheel::ID){
            _mouseData = &_union.wheel->mouse;
         }
         eventBase = &event;
      }
      Internal::InputUnion _union = {};
   public:
      InputEvent(InputEventKey& event)          {assign<std::remove_cvref_t<decltype(event)>>(_union.key          , event);}
      InputEvent(InputEventChar& event)         {assign<std::remove_cvref_t<decltype(event)>>(_union.chr          , event);}
      InputEvent(InputEventMouseMotion& event)  {assign<std::remove_cvref_t<decltype(event)>>(_union.motion       , event);}
      InputEvent(InputEventMouseHover& event)   {assign<std::remove_cvref_t<decltype(event)>>(_union.hover        , event);}
      InputEvent(InputEventMouseButton& event)  {assign<std::remove_cvref_t<decltype(event)>>(_union.button       , event);}
      InputEvent(InputEventMouseWheel& event)   {assign<std::remove_cvref_t<decltype(event)>>(_union.wheel        , event);}
      InputEvent(InputEventController& event)   {assign<std::remove_cvref_t<decltype(event)>>(_union.controller   , event);}
      template <typename T>
      std::optional<const T*> isEvent() const {return eventBase->isEvent<T>();} //get base address
      template <typename T>
      requires std::is_base_of_v<BaseEvent, T>
      constexpr const T& toEvent() const {
         if constexpr (T::ID == InputEventMouseMotion::ID){
            auto member = _union.motion;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventMouseHover::ID){
            auto member = _union.hover;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventMouseButton::ID){
            auto member = _union.button;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventMouseWheel::ID){
            auto member = _union.wheel;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventKey::ID){
            auto member = _union.key;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventChar::ID){
            auto member = _union.chr;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
         if constexpr (T::ID == InputEventController::ID){
            auto member = _union.controller;
            static_assert(std::is_same_v<decltype(member), T*>);
            return static_cast<const T&>(*member);
         }
      }
      //if this is a mouse event, return the mousedata, otherwise return an optional nullptr
      [[nodiscard]] std::optional<MouseEvent*> isMouse() const {return _mouseData ? _mouseData : std::optional<MouseEvent*>();}
      EventId eventId;
   private:
      MouseEvent* _mouseData = nullptr;
      BaseEvent* eventBase;
   };

   enum class InputFilter {
      PASS_ONLY,
      PUBLISH_ONLY,
      PROCESS_ONLY,
      PASS_AND_PROCESS, //passes input to children and then process it locally if it's not handled by a child
      PASS_AND_PUBLISH, //passes input to children and then publishes input to subscribers instead of handling it locally
      PROCESS_AND_PASS, //processes input locally first, and passes it to children if we can't use it locally
      PROCESS_AND_STOP, //processes input locally but does not pass it to children
      IGNORE_AND_PASS, //passes input to children without handling it
      IGNORE_AND_STOP, //ignores input and does not pass or handle it
      PUBLISH_AND_PASS, //publishes input to subscribers instead of handling it locally
      PUBLISH_AND_STOP, //publishes input to subscribers instead of handling it locally or passing it to children
      PASS_PUBLISH_PROCESS, //pass to children, publish, then try to handle locally
      PASS_PROCESS_PUBLISH, //pass to children, process locally, then publish if unhandled (default)
      PROCESS_PUBLISH_PASS,
      PROCESS_PASS_PUBLISH,
      PUBLISH_PASS_PROCESS,
      PUBLISH_PROCESS_PASS,
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
      friend class Window;
   };
}