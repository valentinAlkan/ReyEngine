#include "Window.h"
#include <iostream>
#include "Application.h"
#include "Scene.h"
#include "Canvas.h"
#include "SystemTime.h"
#include "TypeContainer.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
WindowPrototype::WindowPrototype(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS)
: title(title)
, width(width)
, height(height)
, flags(flags)
, targetFPS(targetFPS)
{
    for (const auto& flag : flags){
        switch (flag){
            case Window::Flags::RESIZE:
                SetConfigFlags(FLAG_WINDOW_RESIZABLE);
                break;
            case Window::Flags::IS_EDITOR:
                _isEditor = true;
                break;
        }
    }

    InitWindow(width, height, title.c_str());
    Application::ready();
}

/////////////////////////////////////////////////////////////////////////////////////////
Window& WindowPrototype::createWindow() {
    use();
    return Application::instance().createWindow(*this, nullopt);
}

/////////////////////////////////////////////////////////////////////////////////////////
Window& WindowPrototype::createWindow(std::shared_ptr<Canvas> &root) {
    use();
    return Application::instance().createWindow(*this, root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void WindowPrototype::use() {
    if (_usedUp) {
        throw std::runtime_error(
                "WindowPrototype for window " + title + " {" + std::to_string(width) + ":" + std::to_string(height) + " already used!");
    }
    _usedUp = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, const std::vector<Flags> &flags, int targetFPS)
: NamedInstance("Window", "Window")
, Internal::TypeContainer<BaseWidget>("Window", "Window")
, targetFPS(targetFPS)
, startingWidth(width)
, startingHeight(height)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::initialize(std::optional<std::shared_ptr<Canvas>> optRoot){
    //Create canvas if not provided
    if (!optRoot) {
        optRoot = Canvas::build<Canvas>("root");
    }
    auto& root = optRoot.value();
    root->ReyEngine::Internal::TypeContainer<ReyEngine::BaseWidget>::setRoot(true);
    root->setAnchoring(BaseWidget::Anchor::FILL); //canvas is filled by default
    root->ReyEngine::Internal::TypeContainer<ReyEngine::BaseWidget>::setRoot(true);
    //make sure we init the root
    root->_has_inited = true;
    root->setRect(Rect<int>(0, 0, startingWidth, startingHeight)); //initialize to be the same size as the window
    addChild(root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   //set widgets as processed
   //NOTE: This must be done here, because widgets can be created and loaded before a window exists
   // Since the window controls the process list, it might not exist yet.
   std::function<void(shared_ptr<BaseWidget>)> applyProcess = [&](shared_ptr<BaseWidget> widget){
      for (auto& child : widget->getChildren()){
         applyProcess(child);
      }
      if (widget->_isProcessed.value) widget->setProcess(true);
   };
   applyProcess(getCanvas());
   ReyEngine::Size<int> size = getSize();
   ReyEngine::Pos<int> position;
   Time::RateLimiter rateLimit(targetFPS);
   while (!WindowShouldClose()){
      {
         std::scoped_lock<std::mutex> sl(Application::instance()._busy);
         //process widgets wanting to enter the tree for the first time
//         auto nextPair = Internal::EnterTreeQueue::instance().getNextInQueue();

         //see if the window size has changed
         auto newSize = getSize();
         if (newSize != size) {
            WindowResizeEvent event(toEventPublisher(), newSize);
            size = newSize;
            publish(event);
            //see if our root needs to resize
            if (getCanvas()->getAnchoring() != BaseWidget::Anchor::NONE) {
               getCanvas()->setSize(size);
            }
         }
         //see if the window has moved
         auto newPos = getPosition();
         if (newPos != position) {
            auto me = toEventPublisher();
            WindowMoveEvent event(toEventPublisher());
            position = newPos;
            event.position = newPos;
            publish(event);
         }

         //collect char input (up to limit)
         // only downs for chars - no ups. Use keys for uppies.
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto charDown = InputManager::instance().getCharPressed();
            if (charDown) {
               InputEventChar event(toEventPublisher());
               event.ch = charDown;
               processUnhandledInput(event, nullopt);
            } else {
               break;
            }
         }

         //collect key input (up to limit)
         //do ups first so we don't process up and down on same frame
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto keyUp = InputManager::instance().getKeyReleased();
            if ((int) keyUp) {
               InputEventKey event(toEventPublisher());
               event.key = keyUp;
               event.isDown = false;
               event.isRepeat = false;
               processUnhandledInput(event, nullopt);
            } else {
               break;
            }
         }

         //REPEATS
         auto now = chrono::steady_clock::now();
         static chrono::time_point<chrono::steady_clock> keyDownTimestamp = now;
         auto lastKey = InputManager::getLastKeyPressed();
         if (InputManager::isKeyDown(lastKey)) {
            static chrono::time_point<chrono::steady_clock> keyRepeatTimestamp = now;
            if (now - keyDownTimestamp > _keyDownRepeatDelay) {
               //start sending repeats
               if (now - keyRepeatTimestamp > _keyDownRepeatRate) {
                  keyRepeatTimestamp = chrono::steady_clock::now();
                  InputEventKey event(toEventPublisher());
                  event.key = lastKey;
                  event.isDown = true;
                  event.isRepeat = true;
                  processUnhandledInput(event, nullopt);
               }
            }
         }


         //DOWNS
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto keyDown = InputManager::instance().getKeyPressed();
            if ((int) keyDown) {
               keyDownTimestamp = chrono::steady_clock::now();
               InputEventKey event(toEventPublisher());
               event.key = keyDown;
               event.isDown = true;
               event.isRepeat = false;
               processUnhandledInput(event, nullopt);
            } else {
               break;
            }
         }

         //now do mouse input
         //UPS
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto btnUp = InputManager::instance().getMouseButtonReleased();
            auto pos = InputManager::getMousePos();
            if (btnUp != InputInterface::MouseButton::NONE) {
               if (btnUp == InputInterface::MouseButton::LEFT) {
                  //check for drag n drop
                  if (_dragNDrop.has_value() && _isDragging) {
                     //we have a widget being dragged, lets try to drop it
                     bool handled = false;
                     auto widgetAt = getCanvas()->getWidgetAt(pos);
                     if (widgetAt) {
                        handled = widgetAt.value()->_on_drag_drop(_dragNDrop.value());
                     }
                     _dragNDrop.reset();
                     _isDragging = false;
                     if (handled) continue; //otherwise continue on to publishing an event
                  }
               }

               InputEventMouseButton event(toEventPublisher());
               event.button = btnUp;
               event.isDown = false;
               event.globalPos = pos;
               UnhandledMouseInput mouse;
               mouse.localPos = getCanvas()->globalToLocal(pos);
               mouse.isInside = getCanvas()->isInside(mouse.localPos);
               processUnhandledInput(event, mouse);
            }
         }

         //DOWNS
         for (size_t i = 0; i < Window::INPUT_COUNT_LIMIT; i++) {
            auto btnDown = InputManager::instance().getMouseButtonPressed();
            if (btnDown != InputInterface::MouseButton::NONE) {
               auto pos = InputManager::getMousePos();
               //check for dragndrops
               if (btnDown == InputInterface::MouseButton::LEFT) {
                  auto widgetAt = getCanvas()->getWidgetAt(pos);
                  if (widgetAt) {
                     auto willDrag = widgetAt.value()->_on_drag_start(pos);
                     if (willDrag) {
                        _dragNDrop = willDrag.value();
                        _dragNDrop.value()->startPos = pos;
                     } else {
                        _dragNDrop = nullopt;
                     }
                     _isDragging = false;
                  }
               }
               InputEventMouseButton event(toEventPublisher());
               event.button = btnDown;
               event.isDown = true;
               event.globalPos = pos;
               UnhandledMouseInput mouse;
               mouse.localPos = getCanvas()->globalToLocal(pos);
               mouse.isInside = getCanvas()->isInside(mouse.localPos);
               processUnhandledInput(event, mouse);
            } else {
               break;
            }
         }

         {
            auto wheel = InputManager::getMouseWheel();
            if (wheel) {
               InputEventMouseWheel event(toEventPublisher());
               auto pos = InputManager::getMousePos();
               event.globalPos = pos;
               event.wheelMove = wheel;
               UnhandledMouseInput mouse;
               mouse.localPos = getCanvas()->globalToLocal(event.globalPos);
               mouse.isInside = getCanvas()->isInside(mouse.localPos);
               processUnhandledInput(event, mouse);
            }
         }


         //check the mouse delta compared to last frame
         auto mouseDelta = InputManager::getMouseDelta();
         if (mouseDelta) {
            InputEventMouseMotion event(toEventPublisher());
            event.mouseDelta = mouseDelta;
            event.globalPos = InputManager::getMousePos();

            //don't do hovering or mouse input if we're dragging and dropping
            static constexpr unsigned int DRAG_THRESHOLD = 20;
            if (_dragNDrop) {
               //only drag if we've moved the mouse above a certain threshold
               auto dragDelta = _dragNDrop.value()->startPos - getMousePos();
               if (abs(dragDelta.x) > DRAG_THRESHOLD || abs(dragDelta.y) > DRAG_THRESHOLD) {
                  _isDragging = true;
               }
            } else {
               //find out which widget will accept the mouse motion as focus
               auto hovered = getCanvas()->askHover(event.globalPos);
               if (hovered) {
                  setHover(hovered.value());
               } else {
                  clearHover();
               }
//            if (_isEditor) continue;
               UnhandledMouseInput mouse;
               mouse.localPos = getCanvas()->globalToLocal(event.globalPos);
               mouse.isInside = getCanvas()->isInside(mouse.localPos);
               getCanvas()->_process_unhandled_input(event, mouse);
            }
         }

         //process timers and call their callbacks
         SystemTime::processTimers();

         float dt = getFrameDelta();
         //process widget logic
         for (auto &widget: _processList.getList()) {
            widget->_process(dt);
         }

         //draw children on top of their parents

         ReyEngine::Pos<double> texOffset;
         getCanvas()->renderChain(texOffset);

         //draw the drag and drop preview (if any)
         if (_isDragging && _dragNDrop && _dragNDrop.value()->preview) {
            _dragNDrop.value()->preview.value()->setPos(InputManager::getMousePos());
            _dragNDrop.value()->preview.value()->renderChain(texOffset);
         }
         //render the canvas
         BeginDrawing();
         DrawTextureRec(getCanvas()->_renderTarget.getRenderTexture(), {0,0,(float)getCanvas()->_renderTarget.getSize().x, -(float)getCanvas()->_renderTarget.getSize().y},{0, 0}, WHITE);
         EndDrawing();
      } // release scoped lock here
      //wait some time
      rateLimit.wait();
      _frameCounter++;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Window::~Window(){
   CloseWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////
bool Window::setProcess(bool process, std::shared_ptr<BaseWidget> widget) {
   //return if the operation was successful
   return process ? _processList.add(widget) != nullopt : _processList.remove(widget) != nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool Window::isProcessed(const std::shared_ptr<BaseWidget>& widget) const {
   return _processList.find(widget).has_value();
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<shared_ptr<BaseWidget>> Window::ProcessList::add(std::shared_ptr<BaseWidget> widget) {
   lock_guard<mutex> lock(_mtx);
   auto retval = _list.insert(widget);
   if (retval.second){
      return widget;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> Window::ProcessList::remove(std::shared_ptr<BaseWidget> widget) {
   const lock_guard<mutex> lock(_mtx);
   auto it = _list.find(widget);
   if (it != _list.end()){
      //only remove if found;
      _list.erase(it);
      return widget;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
Pos<int> Window::getMousePos(){
   return InputManager::getMousePos();
}

///////////////////////////////////////////////////////////////////////////////////////////
Vec2<double> Window::getMousePct() {
   auto pos = getMousePos();
   auto screenSize = ReyEngine::getScreenSize();
   auto xRange = Vec2<int>(0,(int)screenSize.x);
   auto yRange = Vec2<int>(0,(int)screenSize.y);
   return {xRange.pct(pos.x), yRange.pct(pos.y)};
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> Window::ProcessList::find(const std::shared_ptr<BaseWidget> &widget) const {
   auto it = std::find(_list.begin(), _list.end(), widget);
   if (it != _list.end()){
      return *it;
   }
   return nullopt;
}
///////////////////////////////////////////////////////////////////////////////////////////
//void Window::setCanvas(std::shared_ptr<Canvas>& newRoot) {
//   makeRoot(newRoot, getSize());
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::clearHover() {
   auto locked =  _hovered.lock();
   if (locked) {
      locked->_hovered = false;
      locked->_on_mouse_exit();
   }
   _hovered.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::setHover(std::shared_ptr<BaseWidget>& widget) {
   auto locked =  _hovered.lock();
   if (locked) {
      if (locked !=  widget){
         locked->_hovered = false;
         locked->_on_mouse_exit();
         _hovered.reset();
      } else {
         return;
      }
   }
   _hovered = widget;
   widget->_hovered = true;
   InputInterface::setCursor(widget->cursor);
   widget->_on_mouse_enter();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::weak_ptr<BaseWidget>> Window::getHovered() {
   if (_hovered.expired()) return nullopt;
   return _hovered;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::processUnhandledInput(InputEvent& inputEvent, std::optional<UnhandledMouseInput> mouseInput){
   //first offer up input to modal widget (if any)
   getCanvas()->_process_unhandled_input(inputEvent, mouseInput);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::pushRenderTarget(ReyEngine::RenderTarget& newTarget) {
   if (!renderStack.empty()) {
      renderStack.top()->endRenderMode();
   }
   renderStack.push(&newTarget);
   renderStack.top()->beginRenderMode();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::popRenderTarget() {
   renderStack.top()->endRenderMode();
   renderStack.pop();
   if (!renderStack.empty()) {
      renderStack.top()->beginRenderMode();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Canvas> Window::getCanvas() {
    return getChildren().at(0)->toType<Canvas>();
}