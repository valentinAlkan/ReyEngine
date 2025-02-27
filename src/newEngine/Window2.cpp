#include "Window2.h"
#include <iostream>
#include "Application2.h"
//#include "Scene.h"
#include "InputManager2.h"
#include "Canvas2.h"
//#include "SystemTime.h"
//#include "TypeContainer.h"
//#include "Physics.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
WindowPrototype2::WindowPrototype2(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
: title(title)
, width(width)
, height(height)
, flags(flags)
, targetFPS(targetFPS)
{
   for (const auto& flag : flags){
      switch (flag){
         case WindowFlags::RESIZE:
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
            break;
         case WindowFlags::IS_EDITOR:
            _isEditor = true;
            break;
      }
   }

   InitWindow(width, height, title.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
Window2& WindowPrototype2::createWindow() {
   use();
   return Application2::instance().createWindow(*this, nullopt);
}

///////////////////////////////////////////////////////////////////////////////////////////
//Window2& WindowPrototype2::createWindow(std::shared_ptr<Canvas> &root) {
//   use();
//   return Application2::instance().createWindow(*this, root);
//}

/////////////////////////////////////////////////////////////////////////////////////////
void WindowPrototype2::use() {
   if (_usedUp) {
      throw std::runtime_error(
            "WindowPrototype2 for window " + title + " {" + std::to_string(width) + ":" + std::to_string(height) + " already used!");
   }
   _usedUp = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window2::Window2(const std::string &title, int width, int height, const std::vector<WindowFlags> &flags, int targetFPS)
: targetFPS(targetFPS)
, startingWidth(width)
, startingHeight(height)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void Window2::initialize(std::optional<std::shared_ptr<Canvas>> optRoot){
   //Create canvas if not provided
//   if (!optRoot) {
//      optRoot = Canvas::build("root");
//   }
//   auto& root = optRoot.value();
//   root->ReyEngine::Internal::TypeContainer<ReyEngine::BaseWidget>::setRoot(true);
//   root->setAnchoring(BaseWidget::Anchor::FILL); //canvas is filled by default
//   //make sure we init the root
   _root = Tree::make_node<Canvas>("root");
//   root->_init();
//   root->_has_inited = true;
//   root->setRect(Rect<int>(0, 0, startingWidth, startingHeight)); //initialize to be the same size as the window
//   TypeContainer<BaseWidget>::addChild(root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window2::exec(){
   InputInterface::setExitKey(InputInterface::KeyCode::KEY_ESCAPE);
   //set widgets as processed
   //NOTE: This must be done here, because widgets can be created and loaded before a window exists
   // Since the window controls the process list, it might not exist yet.
//   std::function<void(shared_ptr<BaseWidget>)> applyProcess = [&](shared_ptr<BaseWidget> widget){
//      for (auto& child : widget->getChildren()){
//         applyProcess(child);
//      }
//      if (widget->_isProcessed.value) widget->setProcess(true);
//   };
   auto canvas = _root->ref<Canvas>();
//   applyProcess(canvas);
   Size<float> size = getSize();
   Pos<float> position;
//   canvas->setSize(size);
   SetTargetFPS(targetFPS);
   while (!WindowShouldClose()){
//
//         unique_lock<mutex> sl(Application::instance()._busy);
//
         //see if the window size has changed
         Size<float> newSize = getSize();
         if (newSize != size) {
            EventSubscriber subscriber;
            WindowResizeEvent event(this, getSize());
            size = newSize;
            publish(event);
            // see if our root needs to resize
            canvas->setSize(size);
            if (newSize != canvas->getSize()){
               canvas->setSize(newSize);
            }
         }

         // see if the window has moved
         Pos<float> newPos = getPosition();
         if (newPos != position) {
            WindowMoveEvent event(this, getPosition());
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
         for (size_t i = 0; i < Window2::INPUT_COUNT_LIMIT; i++) {
            auto charDown = InputManager2::instance().getCharPressed();
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
         for (size_t i = 0; i < Window2::INPUT_COUNT_LIMIT; i++) {
            auto keyUp = InputManager2::instance().getKeyReleased();
            if ((int) keyUp) {
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
         auto now = chrono::steady_clock::now();
         static chrono::time_point<chrono::steady_clock> keyDownTimestamp = now;
         auto lastKey = InputManager2::getLastKeyPressed();
         if (InputManager2::isKeyDown(lastKey)) {
            static chrono::time_point<chrono::steady_clock> keyRepeatTimestamp = now;
            if (now - keyDownTimestamp > _keyDownRepeatDelay) {
               //start sending repeats
               if (now - keyRepeatTimestamp > _keyDownRepeatRate) {
                  keyRepeatTimestamp = chrono::steady_clock::now();
                  InputEventKey event(this);
                  event.key = lastKey;
                  event.isDown = true;
                  event.isRepeat = true;
                  canvas->__process_unhandled_input(event);
               }
            }
         }


         //DOWNS
         for (size_t i = 0; i < Window2::INPUT_COUNT_LIMIT; i++) {
            auto keyDown = InputManager2::instance().getKeyPressed();
            if ((int) keyDown) {
               keyDownTimestamp = chrono::steady_clock::now();
               InputEventKey event(this);
               event.key = keyDown;
               event.isDown = true;
               event.isRepeat = false;
               canvas->__process_unhandled_input(event);
            } else {
               break;
            }
         }

         //now do mouse input
         //UPS
         for (size_t i = 0; i < Window2::INPUT_COUNT_LIMIT; i++) {
            auto btnUp = InputManager2::instance().getMouseButtonReleased();
            auto pos = InputManager2::getMousePos();
            if (btnUp != InputInterface::MouseButton::NONE) {
               if (btnUp == InputInterface::MouseButton::LEFT) {
                  //check for drag n drop
//                  if (_dragNDrop.has_value() && _isDragging) {
//                     //we have a widget being dragged, lets try to drop it
//                     bool handled = false;
//                     auto widgetAt = canvas->getWidgetAt(pos);
//                     if (widgetAt) {
//                        handled = widgetAt.value()->_on_drag_drop(_dragNDrop.value());
//                     }
//                     _dragNDrop.reset();
//                     _isDragging = false;
//                     if (handled) continue; //otherwise continue on to publishing an event
//                  }
               }

               InputEventMouseButton event(this, pos.get(), btnUp, false);
               canvas->__process_unhandled_input(event);
            } else {
               break;
            }
         }

         //DOWNS
         for (size_t i = 0; i < Window2::INPUT_COUNT_LIMIT; i++) {
            auto btnDown = InputManager2::instance().getMouseButtonPressed();
            if (btnDown != InputInterface::MouseButton::NONE) {
               auto pos = InputManager2::getMousePos();
               //check for dragndrops
//               if (btnDown == InputInterface::MouseButton::LEFT) {
//                  auto widgetAt = canvas->getWidgetAt(pos);
//                  if (widgetAt) {
//                     auto willDrag = widgetAt.value()->_on_drag_start(pos);
//                     if (willDrag) {
//                        _dragNDrop = willDrag.value();
//                        _dragNDrop.value()->startPos = pos;
//                     } else {
//                        _dragNDrop = nullopt;
//                     }
//                     _isDragging = false;
//                  }
//               }
               InputEventMouseButton event(this, pos.get(), btnDown, true);
               canvas->__process_unhandled_input(event);
            } else {
               break;
            }
         }

         {
            auto wheel = InputManager2::getMouseWheel();
            if (wheel) {
               InputEventMouseWheel event(this, InputManager2::getMousePos().get(), wheel);
               canvas->__process_unhandled_input(event);
            }
         }


//         //check the mouse delta compared to last frame
         auto mouseDelta = InputManager2::getMouseDelta();
         if (mouseDelta) {
            InputEventMouseMotion event(this, InputManager2::getMousePos().get(), mouseDelta);
            event.mouseDelta = mouseDelta;

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
            canvas->__process_unhandled_input(event);
//            }
         }

         //process timers and call their callbacks
//         SystemTime::processTimers();

         float dt = getFrameDelta();
         //process widget logic
//         _processList.processAll(dt);

         //draw the canvas to our render texture
         rlLoadIdentity();
         auto& _renderTarget = canvas->getRenderTarget();
//         Application::getWindow(0).pushRenderTarget(_renderTarget);
//         _renderTarget.clear();
         canvas->renderProcess();
//         Application::getWindow(0).popRenderTarget(); //debug

         //do physics synchronously for now
         rlLoadIdentity();
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
         DrawTextureRec(_renderTarget.getRenderTexture(), {0, 0, (float) _renderTarget.getSize().x, -(float) getSize().y}, {0, 0}, WHITE);
         EndDrawing();
//      } // release scoped lock here
//      _frameCounter++;
   }
//   _processList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
Window2::~Window2(){
   Logger::debug() << "Deleting Window" << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////
//bool Window2::setProcess(bool process, std::shared_ptr<BaseWidget> widget) {
//   //return if the operation was successful
//   return process ? _processList.add(widget) != nullopt : _processList.remove(widget) != nullopt;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//bool Window2::isProcessed(const std::shared_ptr<BaseWidget>& widget) const {
//   return _processList.find(widget).has_value();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//std::optional<shared_ptr<BaseWidget>> Window2::ProcessList::add(std::shared_ptr<BaseWidget>& widget) {
//   unique_lock<mutex> lock(_mtx);
//   auto retval = _list.insert(widget);
//   if (retval.second){
//      return widget;
//   }
//   return nullopt;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//std::optional<std::shared_ptr<BaseWidget>> Window2::ProcessList::remove(std::shared_ptr<BaseWidget>& widget) {
//   unique_lock<mutex> lock(_mtx);
//   auto it = _list.find(widget);
//   if (it != _list.end()){
//      //only remove if found;
//      _list.erase(it);
//      return widget;
//   }
//   return nullopt;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//void Window2::ProcessList::processAll(double dt) {
//   unique_lock<mutex> lock(_mtx);
//   for (auto &widget : _list) {
//      widget->_process(dt);
//   }
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//WindowSpace<Pos<float>> Window2::getMousePos(){
//   return InputManager2::getMousePos();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//Vec2<double> Window2::getMousePct() {
//   auto pos = getMousePos().get();
//   auto screenSize = ReyEngine::getScreenSize();
//   auto xRange = Vec2<int>(0,(int)screenSize.x);
//   auto yRange = Vec2<int>(0,(int)screenSize.y);
//   return {xRange.pct(pos.x).get(), yRange.pct(pos.y).get()};
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//std::optional<std::shared_ptr<BaseWidget>> Window2::ProcessList::find(const std::shared_ptr<BaseWidget> &widget) const {
//   auto it = std::find(_list.begin(), _list.end(), widget);
//   if (it != _list.end()){
//      return *it;
//   }
//   return nullopt;
//}
/////////////////////////////////////////////////////////////////////////////////////////////
////void Window2::setCanvas(std::shared_ptr<Canvas>& newRoot) {
////   makeRoot(newRoot, getSize());
////}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//void Window2::setSize(Size<int> newSize) {
//   setWindowSize(newSize);
//   getCanvas()->setSize(newSize);
//}
///////////////////////////////////////////////////////////////////////////////////////////
//void Window2::clearHover() {
//   auto locked =  _hovered.lock();
//   if (locked) {
//      locked->_hovered = false;
//      locked->_on_mouse_exit();
//   }
//   _hovered.reset();
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void Window2::setHover(std::shared_ptr<BaseWidget>& widget) {
//   auto locked =  _hovered.lock();
//   if (locked) {
//      if (locked !=  widget){
//         locked->_hovered = false;
//         locked->_on_mouse_exit();
//         _hovered.reset();
//      } else {
//         return;
//      }
//   }
//   _hovered = widget;
//   widget->_hovered = true;
//   InputInterface::setCursor(widget->cursor);
//   widget->_on_mouse_enter();
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::optional<std::weak_ptr<BaseWidget>> Window2::getHovered() {
//   if (_hovered.expired()) return nullopt;
//   return _hovered;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void Window2::pushRenderTarget(ReyEngine::RenderTarget& newTarget) {
//   if (!renderStack.empty()) {
//      renderStack.top()->endRenderMode();
//   }
//   renderStack.push(&newTarget);
//   renderStack.top()->beginRenderMode();
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void Window2::popRenderTarget() {
//   renderStack.top()->endRenderMode();
//   renderStack.pop();
//   if (!renderStack.empty()) {
//      renderStack.top()->beginRenderMode();
//   }
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Canvas> Window2::getCanvas() {
   return _root->ref<Canvas>();
}