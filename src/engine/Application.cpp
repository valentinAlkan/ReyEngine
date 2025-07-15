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
Window& Application::createWindow(Internal::WindowPrototype& prototype, std::optional<std::shared_ptr<Canvas>> root){
   _windows.emplace_back(new Window(prototype.title, prototype.width, prototype.height, prototype.flags, prototype.targetFPS));
   auto& window = *_windows.back();
   window.initialize(root);
//   window.getCanvas()->setRect({}); //will auto fill
   return window;
}

/////////////////////////////////////////////////////////////////////////////////////////
//void Application2::registerForApplication2Ready(std::function<void()> fx) {
//   if (!isReady()) {
//      instance()._applicationReadyList.push_back(fx);
//   }
//}

/////////////////////////////////////////////////////////////////////////////////////////
//void Application2::registerForApplication2Ready(std::function<void()> cb) {
//   if (!isReady()) {
//      instance()._initListArbCallback.push_back(cb);
//   } else {
//      cb();
//   }
//}

///////////////////////////////////////////////////////////////////////////////////////////
//void Application2::ready() {
//   for (auto& component : instance()._applicationReadyList){
//      component->_on_application_ready();
//   }
//   instance()._applicationReadyList.clear(); //done with this forever
//   for (auto& cb : instance()._initListArbCallback){
//      cb();
//   }
//   instance()._initListArbCallback.clear(); //done with this forever
//   instance()._is_ready = true;
//}

////////////////////////////////////////////////////////////////////////////////////////
std::unique_lock<std::mutex> Application::getLock() {
   std::unique_lock<std::mutex> l(instance()._busy);
   return std::move(l);
}

////////////////////////////////////////////////////////////////////////////////////////
long double Application::secondsSinceInit() {
   return std::chrono::duration<long double>(std::chrono::steady_clock::now() - instance()._startTime).count();
}
