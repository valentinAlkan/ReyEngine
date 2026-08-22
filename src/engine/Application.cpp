#include "Window.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
Application::Application()
{
   _startTime = chrono::steady_clock::now();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<Internal::WindowPrototype> Application::createWindowPrototype(const std::string &title, int width, int height, const std::vector<ReyEngine::WindowFlags> &flags, int targetFPS) {
   Application::instance(); //initalize application at least once
   return std::unique_ptr<WindowPrototype>(new WindowPrototype(title, width, height, flags, targetFPS));
}
/////////////////////////////////////////////////////////////////////////////////////////
Window& Application::createWindow(Internal::WindowPrototype& prototype){
   _windows.emplace_back(new Window(prototype.title, prototype.width, prototype.height, prototype.flags, prototype.targetFPS));
   auto& window = *_windows.back();
   std::optional<std::unique_ptr<TypeNode>> root;
   if (prototype.root) root = std::move(prototype.root);
   window.initialize(std::move(root));
//   window.getCanvas()->setRect({}); //will auto fill
   return window;
}

////////////////////////////////////////////////////////////////////////////////////////
std::unique_lock<std::mutex> Application::getLock() {
   std::unique_lock<std::mutex> l(instance()._busy);
   return std::move(l);
}

////////////////////////////////////////////////////////////////////////////////////////
long double Application::secondsSinceInit() {
   return std::chrono::duration<long double>(std::chrono::steady_clock::now() - instance()._startTime).count();
}
