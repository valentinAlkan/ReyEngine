#include "Application.h"

#include <utility>

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Window> Application::createWindow(const std::string &title, int width, int height, std::shared_ptr<BaseWidget> root, const std::vector<Window::Flags> &flags, int targetFPS){
   if (window){
      return nullptr;
   }
   return std::shared_ptr<Window>(new Window("MainWindow", width, height, std::move(root), flags, targetFPS));
}