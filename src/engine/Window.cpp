#include "Window.h"
#include <iostream>
#include "Application.h"
//#include "Scene.h"
#include "InputManager.h"
#include "Canvas.h"
//#include "SystemTime.h"
//#include "TypeContainer.h"
//#include "Physics.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using sc = chrono::steady_clock;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
: targetFPS(targetFPS)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::initialize(std::optional<std::shared_ptr<Canvas>> optRoot){
   //Create canvas if not provided
//   if (!optRoot) {
//      optRoot = Canvas::build("root");
//   }
//   auto& root = optRoot.value();
//   root->ReyEngine::Internal::TypeContainer<ReyEngine::Widget>::setRoot(true);
//   root->setAnchoring(Widget::Anchor::FILL); //canvas is filled by default
//   //make sure we init the root
   auto [canvas, node] = make_node<Canvas>("root");
   _root = std::move(node);
   canvas->setSize(getSize());
   canvas->window = this;
//   root->_init();
//   root->_has_inited = true;
//   root->setRect(Rect<int>(0, 0, startingWidth, startingHeight)); //initialize to be the same size as the window
//   TypeContainer<Widget>::addChild(root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   InputInterface::setExitKey(InputInterface::KeyCode::KEY_ESCAPE);
   auto canvas = _root->ref<Canvas>();
   Size<float> size = getSize();
   Pos<float> position;
   SetTargetFPS(targetFPS);
   auto inputEventMouseButtonTimeStampUp = sc::now();
   auto inputEventMouseButtonTimeStampDown = sc::now();
   auto lastMouseButtonInput = InputInterface::MouseButton::NONE;
   while (!WindowShouldClose()){
      //see if the window size has changed
      Size<float> newSize = getSize();
      if (newSize != size) {
         EventSubscriber subscriber;
         WindowResizeEvent event(this, getSize());
         size = newSize;
         publish(event);
         //root canvas always same size as window
         canvas->setSize(size);
      }

      // see if the window has moved
      Pos<R_FLOAT> newPos = Pos<R_FLOAT>(getPosition());
      if (newPos != position) {
         WindowMoveEvent event(this, Pos<R_FLOAT>(getPosition()));
         position = newPos;
         event.position = newPos;
         publish(event);
      }

      // programatically generated inputs
      while(!_inputQueueKey.empty()){
         auto event = std::move(_inputQueueKey.front());
         _inputQueueKey.pop();
         canvas->__process_unhandled_input(*event);
      }

      while(!_inputQueueMouse.empty()){
         auto event = std::move(_inputQueueMouse.front());
         _inputQueueMouse.pop();
         canvas->__process_unhandled_input(*event);
      }

      // collect char input (up to limit)
      // only downs for chars - no ups. Use keys for uppies.
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto charDown = InputManager::instance().getCharPressed();
         if (charDown) {
            InputEventChar event(this);
            event.ch = charDown;
            canvas->__process_unhandled_input(InputEvent(event));
         } else {
            break;
         }
      }

      //collect key input (up to limit)
      //do ups first so we don't process up and down on same frame
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto keyUp = InputManager::instance().getKeyReleased();
         if (keyUp != InputInterface::KeyCode::KEY_NULL) {
            InputEventKey event(this);
            event.key = keyUp;
            event.isDown = false;
            event.isRepeat = false;
            canvas->__process_unhandled_input(InputEvent(event));
         } else {
            break;
         }
      }

      //REPEATS
      {
         auto now = sc::now();
         static auto keyDownTimestamp = now;
         auto lastKey = InputManager::getLastKeyPressed();
         if (InputManager::isKeyDown(lastKey)) {
            static auto keyRepeatTimestamp = now;
            if (now - keyDownTimestamp > _keyDownRepeatDelay) {
               //start sending repeats
               if (now - keyRepeatTimestamp > _keyDownRepeatRate) {
                  keyRepeatTimestamp = sc::now();
                  InputEventKey event(this);
                  event.key = lastKey;
                  event.isDown = true;
                  event.isRepeat = true;
                  canvas->__process_unhandled_input(event);
               }
            }
         }

         //DOWNS
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto keyDown = InputManager::instance().getKeyPressed();
            if (keyDown != InputInterface::KeyCode::KEY_NULL) {
               keyDownTimestamp = sc::now();
               InputEventKey event(this);
               event.key = keyDown;
               event.isDown = true;
               event.isRepeat = false;
               canvas->__process_unhandled_input(event);
            } else {
               break;
            }
         }
      }

      //now do mouse input
      //UPS
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto btnUp = InputManager::instance().getMouseButtonReleased();
         if (btnUp != InputInterface::MouseButton::NONE) {
            auto now = sc::now();
            bool isDouble = (now - inputEventMouseButtonTimeStampUp) < _doubleClickThreshold && lastMouseButtonInput == btnUp;
            inputEventMouseButtonTimeStampUp = now;
            lastMouseButtonInput = btnUp;
            auto pos = InputManager::getMousePos();
            InputEventMouseButton event(this, pos.get(), btnUp, false, isDouble);
            if (isDouble) inputEventMouseButtonTimeStampUp = sc::time_point{};
            canvas->__process_unhandled_input(event);
         } else {
            break;
         }
      }

      //DOWNS
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto btnDown = InputManager::instance().getMouseButtonPressed();
         if (btnDown != InputInterface::MouseButton::NONE) {
            auto now = sc::now();
            bool isDouble = (now - inputEventMouseButtonTimeStampDown) < _doubleClickThreshold && lastMouseButtonInput == btnDown;
            inputEventMouseButtonTimeStampDown = now;
            lastMouseButtonInput = btnDown;
            auto pos = InputManager::getMousePos();
            InputEventMouseButton event(this, pos.get(), btnDown, true, isDouble);
            canvas->__process_unhandled_input(event);
            if (isDouble) inputEventMouseButtonTimeStampDown = sc::time_point{};
         } else {
            break;
         }
      }

      {
         auto wheel = InputManager::getMouseWheel();
         if (wheel) {
            InputEventMouseWheel event(this, InputManager::getMousePos().get(), wheel);
            canvas->__process_unhandled_input(event);
         }
      }


//         //check the mouse delta compared to last frame
      auto mouseDelta = InputManager::getMouseDelta();
      if (mouseDelta) {
         auto pos = InputManager::getMousePos().get();
         InputEventMouseHover hoverEvent(this, pos);
         InputEventMouseMotion motionEvent(this, pos, mouseDelta);

         //don't do hovering or mouse input if we're dragging and dropping
//            static constexpr unsigned int DRAG_THRESHOLD = 20;
//            if (_dragNDrop) {
         //only drag if we've moved the mouse above a certain threshold
//               auto dragDelta = _dragNDrop.value()->startPos - getMousePos();
//               if (abs(dragDelta.x) > DRAG_THRESHOLD || abs(dragDelta.y) > DRAG_THRESHOLD) {
//                  _isDragging = true;
//               }
//            } else {
         //find out which widget will accept the mouse motion as focus
//               auto hovered = canvas->askHover(canvas->screenToWorld(event.canvasPos));
//               if (hovered) {
//                  setHover(hovered.value());
//               } else {
//                  clearHover();
//               }
//            if (_isEditor) continue;
         if (!canvas->__process_unhandled_input(motionEvent)) {
            canvas->__process_hover(hoverEvent);
         }
//            }
      }

      //process timers and call their callbacks
//         SystemTime::processTimers();

      //process logic
      float dt = getFrameDelta();
      _processList.processAll(dt);

      //draw the canvas to our render texture
//         Application::getWindow(0).pushRenderTarget(_renderTarget);
//         _renderTarget.clear();
//         rlLoadIdentity();
//         rlPushMatrix();
      canvas->renderProcess(canvas->_renderTarget);
      //after invoking the normal render process, we have to draw foreground objects
//         for (auto& foregroudChild : canvas->_foreground.getValues()){
//            canvas->processNode<Canvas::RenderProcess>(foregroudChild, false);
//         }
//         rlPopMatrix();
//         Application::getWindow(0).popRenderTarget(); //debug

      //do physics synchronously for now
//         rlLoadIdentity();
//         Application::getWindow(0).pushRenderTarget(_renderTarget); //debug
//         Physics::PhysicsSystem::process();
//         Application::getWindow(0).popRenderTarget(); //debug

//
//         //draw the drag and drop preview (if any)
////         if (_isDragging && _dragNDrop && _dragNDrop.value()->preview) {
////            _dragNDrop.value()->preview.value()->setPos(InputManager2::getMousePos().get());
////            _dragNDrop.value()->preview.value()->render2DChain();
////         }
      //render the canvas to the window

         BeginDrawing();
         auto& _renderTarget = canvas->getRenderTarget();
         Rect<R_FLOAT> rect = getSize().toRect();
         drawRenderTargetRect(_renderTarget, rect, rect, Colors::none);
         EndDrawing();
         _frameCounter++;
//      } // release scoped lock here
   }
   _processList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
Window::~Window(){
   Logger::debug() << "Deleting Window" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Window::setProcess(bool process, ReyEngine::Internal::Processable* processable) {
   //return if the operation was successful
   return process ? _processList.add(processable) != nullopt : _processList.remove(processable) != nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool Window::isProcessed(const ReyEngine::Internal::Processable* processable) const {
   return _processList.find(processable).has_value();
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<ReyEngine::Internal::Processable*> Window::ProcessList::add(ReyEngine::Internal::Processable* processable) {
   unique_lock<mutex> lock(_mtx);
   auto retval = _list.insert(processable);
   if (retval.second){
      return processable;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<ReyEngine::Internal::Processable*> Window::ProcessList::remove(ReyEngine::Internal::Processable* processable) {
   unique_lock<mutex> lock(_mtx);
   auto it = _list.find(processable);
   if (it != _list.end()){
      //only remove if found;
      _list.erase(it);
      return processable;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
void Window::ProcessList::processAll(R_FLOAT dt) {
   unique_lock<mutex> lock(_mtx);
   for (auto& processable : _list) {
      processable->_process(dt);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<ReyEngine::Internal::Processable*> Window::ProcessList::find(const ReyEngine::Internal::Processable* processable) const {
   auto it = std::find(_list.begin(), _list.end(), processable);
   if (it != _list.end()){
      return *it;
   }
   return nullopt;
}
/////////////////////////////////////////////////////////////////////////////////////////////
////void Window::setCanvas(std::shared_ptr<Canvas>& newRoot) {
////   makeRoot(newRoot, getSize());
////}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//void Window::setSize(Size<int> newSize) {
//   setWindowSize(newSize);
//   getCanvas()->setSize(newSize);
//}

///////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Canvas> Window::getCanvas() {
   return _root->ref<Canvas>();
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Widget *> Window::processInput(const InputEvent& event) {
   // live dangerously. (roots MUST be canvases or you will crash and burn and die too probably)
   auto handler = ReyEngine::Internal::Tree::ProtectedFunctionAccessor(_root.get()).dangerousIs<Canvas>()->__process_unhandled_input(event);
   if (handler) return handler;
   return {};
}