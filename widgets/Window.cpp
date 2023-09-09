#include "Window.h"
#include <iostream>
#include <utility>

using namespace std;

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
   while (!WindowShouldClose()){
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

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//WindowCallback::WindowCallback(std::function<void(float)> callback, const std::string &title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Flags> &flags, int targetFPS)
//: Window(title, width, height, std::move(root), flags, targetFPS)
//, _callback(std::move(callback)){}
//
///////////////////////////////////////////////////////////////////////////////////////////
//WindowCallback::~WindowCallback() noexcept {
////   CloseWindow(); //double free if you call this? window allocated on stack somewhere in raylib prolly
//   cout << "Goodbye!" << endl;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void WindowCallback::exec() {
//   while (!WindowShouldClose()){
//      _callback(GetFrameTime());
//   }
//}