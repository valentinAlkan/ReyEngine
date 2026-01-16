#include "Window.h"
#include <iostream>
#include "Application.h"
#include "InputManager.h"
#include "Canvas.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using sc = chrono::steady_clock;

constexpr bool PRINT_MOUSEUP = false;
constexpr bool PRINT_MOUSEDOWN = false;
constexpr bool PRINT_HOVER= false;
constexpr bool PRINT_MOTION = false;
constexpr bool PRINT_TOOLTIP = true;
constexpr bool PRINT_WHEEL = false;

constexpr std::chrono::milliseconds TOOLTIP_DELAY = 500ms;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
: _targetFPS(targetFPS)
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
   SetExitKey(KEY_NULL);
//   root->_init();
//   root->_has_inited = true;
//   root->setRect(Rect<int>(0, 0, startingWidth, startingHeight)); //initialize to be the same size as the window
//   TypeContainer<Widget>::addChild(root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   auto canvas = _root->ref<Canvas>();
   Size<float> size;
   Pos<float> position;
   WindowSpace<Pos<float>> mousePos = {{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()}};
   Vec2<float> mouseDelta;
   bool checkedToolTip = false;
   SetTargetFPS(_targetFPS);
   auto inputEventMouseButtonTimeStampUp = sc::now();
   auto inputEventMouseButtonTimeStampDown = sc::now();
   auto lastMouseButtonInput = InputInterface::MouseButton::NONE;
   std::chrono::steady_clock::time_point mousePosChangeTime;
   publish(WindowExecEvent(this));
   while (!WindowShouldClose()){
      //see if the window size has changed
      Size<float> newSize = getSize();
      if (newSize != size) {
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
      mouseDelta = InputManager::getMouseDelta();
      mousePos = InputManager::getMousePos();
      for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
         auto btnUp = InputManager::instance().getMouseButtonReleased();
         if (btnUp != InputInterface::MouseButton::NONE) {
            auto now = sc::now();
            bool isDouble = (now - inputEventMouseButtonTimeStampUp) < _doubleClickThreshold && lastMouseButtonInput == btnUp;
            inputEventMouseButtonTimeStampUp = now;
            lastMouseButtonInput = btnUp;
            InputEventMouseButton event(this, mousePos.get(), btnUp, false, isDouble);
            if (isDouble) inputEventMouseButtonTimeStampUp = sc::time_point{};
            auto handledBy = canvas->__process_unhandled_input(event);
            if constexpr (PRINT_MOUSEUP) if (handledBy) Logger::info() << "MouseUp handled by " << handledBy->getName() << endl;
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
            InputEventMouseButton event(this, mousePos.get(), btnDown, true, isDouble);
            auto handledBy = canvas->__process_unhandled_input(event);
            if constexpr (PRINT_MOUSEDOWN) if (handledBy) Logger::info() << "MouseDown handled by " << handledBy->getName() << endl;
            if (isDouble) inputEventMouseButtonTimeStampDown = sc::time_point{};
         } else {
            break;
         }
      }

      {
         auto wheel = InputManager::getMouseWheel();
         if (wheel) {
            InputEventMouseWheel event(this, mousePos.get(), wheel);
            if constexpr (PRINT_WHEEL) Logger::info() << "Sending mouse wheel event " << event.wheelMove << endl;
            auto handledBy = canvas->__process_unhandled_input(event);
            if constexpr (PRINT_WHEEL) if (handledBy) Logger::info() << "Mouse wheel handled by " << handledBy->getName() << endl;
         }
      }


      //check the mouse delta compared to last frame
      if (mouseDelta) {
         checkedToolTip = false;
         mousePosChangeTime = std::chrono::steady_clock::now();
         InputEventMouseHover hoverEvent(this, mousePos.get());
         InputEventMouseMotion motionEvent(this, mousePos.get(), mouseDelta);

         auto handledBy = canvas->__process_unhandled_input(motionEvent);
         if constexpr (PRINT_MOTION) if (handledBy) Logger::info() << "Motion handled by " << handledBy->getName() << endl;
         if (!handledBy) {
            handledBy = canvas->__process_hover(hoverEvent);
            if constexpr (PRINT_HOVER) if (handledBy) Logger::info() << "Hover handled by " << handledBy->getName() << endl;
         }
//            }
      }

      //check for tooltips
      if (!checkedToolTip && canvas->getRect().contains(mousePos.get()) && std::chrono::steady_clock::now() - mousePosChangeTime > TOOLTIP_DELAY){
         InputEventMouseToolTip tooltipEvent(this, mousePos.get());
         checkedToolTip = true;
         auto handledBy = canvas->__process_unhandled_input(tooltipEvent);
         if constexpr (PRINT_TOOLTIP) if (handledBy) Logger::info() << "Tooltip handled by " << handledBy->getName() << endl;
      }

      //process timers and call their callbacks
      //SystemTime::processTimers();

      //process logic
      float dt = getFrameDelta();
      ProcessList<Internal::Tree::Processable>::processAll(dt);
      //process and reap easings
      std::vector<Easing*> toReap;
      if (auto first = ProcessList<Easing>::processfirst(dt)){
         if (first->done()) toReap.push_back(first);
      }
      while(auto easing = ProcessList<Easing>::processNext(dt)){
         if (easing->done()) toReap.push_back(easing);
      }
      for (auto& reap : toReap){
         reap->easable()->removeEasing(reap);
      }


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
   _isClosing = true;
   ProcessList<Internal::Tree::Processable>::clear();
   ProcessList<Easing>::clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
Window::~Window(){
   Logger::debug() << "Deleting Window" << endl;
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