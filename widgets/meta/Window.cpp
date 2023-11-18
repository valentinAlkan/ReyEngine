#include "Window.h"
#include <iostream>
#include <utility>
#include <array>
#include "Application.h"

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, const std::vector<Flags>& flags, int targetFPS)
{

   for (const auto& flag : flags){
      switch (flag){
         case Flags::RESIZE:
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
            break;
         case Flags::IS_EDITOR:
            _isEditor = true;
            break;
      }
   }

   InitWindow(width, height, title.c_str());
   SetTargetFPS(targetFPS);
   Application::ready();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   if (!_root){
      throw std::runtime_error("Window does not have a root! Did you call setRoot()? I bet not. That's what I bet.");
   }
   //set widgets as processed
   //NOTE: This must be done here, because widgets can be created and loaded before a window exists
   // Since the window controls the process list, it might not exist yet.
   std::function<void(shared_ptr<BaseWidget>)> applyProcess = [&](shared_ptr<BaseWidget> widget){
      for (auto& [name, childIter] : widget->_children){
         applyProcess(childIter.second);
      }
      if (widget->_isProcessed.value) widget->setProcess(true);
   };
   applyProcess(_root);
   GFCSDraw::Size<int> size;
   GFCSDraw::Pos<int> position;
   while (!WindowShouldClose()){
      //process widgets wanting to enter the tree for the first time
      Application::processEnterTree();

      //see if the window size has changed
      auto newSize = getSize();
      if (newSize != size){
         WindowResizeEvent event(toEventPublisher(), newSize);
         size = newSize;
         publish(event);
      }
      //see if the window has moved
      auto newPos = getPosition();
      if (newPos != position){
         WindowMoveEvent event(inheritable_enable_shared_from_this<EventPublisher>::shared_from_this());
         position = newPos;
         event.position = newPos;
         publish(event);
      }

      //collect key input (up to limit)
      //do ups first so we don't process up and down on same frame
      for (size_t i=0; i<Window::INPUT_COUNT_LIMIT; i++){
         auto keyUp = InputManager::instance().getKeyReleased();
         if (keyUp){
            InputEventKey event(nullptr);
            event.key = keyUp;
            event.isDown = false;
            _root->_process_unhandled_input(event);
         } else {
            break;
         }
      }

      //DOWNS
      for (size_t i=0; i<Window::INPUT_COUNT_LIMIT; i++){
         auto keyDown = InputManager::instance().getKeyPressed();
         if (keyDown){
            InputEventKey event(nullptr);
            event.key = keyDown;
            event.isDown = true;
            _root->_process_unhandled_input(event);
         } else {
            break;
         }
      }

      //now do mouse input
      //UPS
      for (size_t i=0; i<Window::INPUT_COUNT_LIMIT; i++){
         auto btnUp = InputManager::instance().getMouseButtonReleased();
         if (btnUp != InputInterface::MouseButton::MOUSE_BUTTON_NONE){
            InputEventMouseButton event(nullptr);
            event.button = btnUp;
            event.isDown = false;
            event.globalPos = InputManager::getMousePos();
            _root->_process_unhandled_input(event);
         } else {
            break;
         }
      }

      //DOWNS
      for (size_t i=0; i<Window::INPUT_COUNT_LIMIT; i++){
         auto btnDown = InputManager::instance().getMouseButtonPressed();
         if (btnDown != InputInterface::MouseButton::MOUSE_BUTTON_NONE){
            InputEventMouseButton event(nullptr);
            event.button = btnDown;
            event.isDown = true;
            event.globalPos = InputManager::getMousePos();
            _root->_process_unhandled_input(event);
         } else {
            break;
         }
      }

      //check the mouse delta compared to last frame
      auto mouseDelta = InputManager::getMouseDelta();
      if (mouseDelta) {
         InputEventMouseMotion event(nullptr);
         event.mouseDelta = mouseDelta;
         event.globalPos = InputManager::getMousePos();
         _root->_process_unhandled_input(event);

         //find out which widget will accept the mouse motion as focus
         std::function<std::optional<std::shared_ptr<BaseWidget>>(const std::shared_ptr<BaseWidget>&)> askHover = [&](const std::shared_ptr<BaseWidget>& widget)->std::optional<std::shared_ptr<BaseWidget>>{
            //ask this widget to accept the hover
            auto isInside = [&](const std::shared_ptr<BaseWidget>& widget){
               return widget->getRect().toSizeRect().isInside(widget->globalToLocal(event.globalPos));
            };

            auto process = [&](const std::shared_ptr<BaseWidget>& widget) -> std::optional<std::shared_ptr<BaseWidget>> {
               bool _isInside = isInside(widget);
               if (widget->acceptsHover && _isInside) {
                  return widget;
               }
               return nullopt;
            };

            auto pass = [&](const std::shared_ptr<BaseWidget>& widget) -> std::optional<std::shared_ptr<BaseWidget>>{
               for (const auto& child : widget->getChildren()){
                  auto handled = askHover(child);
                  if (handled) return handled;
               }
               return nullopt;
            };
//            Application::printDebug() << "Asking widget " << widget->getName() << " to accept hover " << endl;
            std::optional<std::shared_ptr<BaseWidget>> handled;
            switch (widget->inputFilter) {
               case InputFilter::INPUT_FILTER_PROCESS_AND_STOP:
                  return process(widget);
               case InputFilter::INPUT_FILTER_PROCESS_AND_PASS:
                  handled = process(widget);
                  if (handled) return handled;
                  return pass(widget);
               case InputFilter::INPUT_FILTER_PASS_AND_PROCESS:
                  handled = pass(widget);
                  if (handled) return handled;
                  return process(widget);
               case InputFilter::INPUT_FILTER_IGNORE_AND_STOP:
                  return nullopt;
               case InputFilter::INPUT_FILTER_IGNORE_AND_PASS:
                  return pass(widget);
            }
         };
         auto hovered = askHover(_root);
         if (hovered) Application::setHover(hovered.value()); else Application::clearHover();
      }

      //process timers and call their callbacks
      SystemTime::processTimers();

      float dt = getFrameDelta();
      //process widget logic
      for (auto& widget : _processList.getList()){
         widget->_process(dt);
      }

      //draw children on top of their parents
      BeginDrawing();
      ClearBackground(RAYWHITE);
      GFCSDraw::Pos<double> texOffset;
      _root->renderChain(texOffset);
      EndDrawing();
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
   auto screenSize = GFCSDraw::getScreenSize();
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
void Window::setRoot(std::shared_ptr<BaseWidget>& newRoot) {
   if (!newRoot->_has_inited){
      newRoot->_init();
   }
   _root = newRoot;
}