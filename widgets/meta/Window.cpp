#include "Window.h"
#include <iostream>
#include <utility>
#include "DrawInterface.h"

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string &title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Flags>& flags, int targetFPS)
: _root(std::move(root))
{
   if (!_root){
      std::runtime_error("Window root cannot be null!");
   }

   for (const auto& flag : flags){
      switch (flag){
         case Flags::RESIZE:
            SetConfigFlags(FLAG_WINDOW_RESIZABLE);
            break;
      }
   }

   InitWindow(width, height, title.c_str());
   SetTargetFPS(targetFPS);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Window::exec(){
   //set widgets as processed
   //NOTE: This must be done here, because widgets can be created loaded and created before a window exists
   // Since the window controls the process list, it might not exist yet.
   std::function<void(shared_ptr<BaseWidget>)> applyProcess = [&](shared_ptr<BaseWidget> widget){
      for (auto& [name, child] : widget->_children){
         applyProcess(child);
      }
      if (widget->_isProcessed.value) widget->setProcess(true);
   };
   applyProcess(_root);

   while (!WindowShouldClose()){
      //collect input

      float dt = getFrameDelta();
      //process widget logic
      for (auto& widget : _processList.getList()){
         widget->_process(dt);
      }

      //draw children on top of their parents
      BeginDrawing();
      ClearBackground(RAYWHITE);
      _root->renderChildren();
      _root->render();
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
Vec2<int> Window::getMousePos(){
   return GFCSDraw::getMousePos();
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> Window::ProcessList::find(const std::shared_ptr<BaseWidget> &widget) const {
   auto it = std::find(_list.begin(), _list.end(), widget);
   if (it != _list.end()){
      return *it;
   }
   return nullopt;
}