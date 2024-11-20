#include "Application.h"
#include "Component.h"
#include <utility>
#include <random>
#include "Canvas.h"
#include "Platform.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
Application::Application()
{
   _startTime = chrono::steady_clock::now();
   TypeManager::instance()._registerTypes();
}

/////////////////////////////////////////////////////////////////////////////////////////
Internal::WindowPrototype Application::createWindowPrototype(const std::string &title, int width, int height, const std::vector<ReyEngine::Window::Flags> &flags, int targetFPS) {
   Application::instance(); //initalize application at least once
    return WindowPrototype(title, width, height, flags, targetFPS);
}
/////////////////////////////////////////////////////////////////////////////////////////
Window& Application::createWindow(Internal::WindowPrototype& prototype, std::optional<std::shared_ptr<Canvas>> root){
   _windows.emplace_back(new Window(prototype.title, prototype.width, prototype.height, prototype.flags, prototype.targetFPS));
   auto& window = *_windows.back();
   window.initialize(root);
   window.getCanvas()->setRect({}); //will auto fill
   return window;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForApplicationReady(std::shared_ptr<Component>& component) {
   if (!isReady()) {
      instance()._applicationReadyList.insert(component);
   } else {
      component->_on_application_ready();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForApplicationReady(std::function<void()> cb) {
   if (!isReady()) {
      instance()._initListArbCallback.push_back(cb);
   } else {
      cb();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::ready() {
   for (auto& component : instance()._applicationReadyList){
      component->_on_application_ready();
   }
   instance()._applicationReadyList.clear(); //done with this forever
   for (auto& cb : instance()._initListArbCallback){
      cb();
   }
   instance()._initListArbCallback.clear(); //done with this forever
   instance()._is_ready = true;
}

////////////////////////////////////////////////////////////////////////////////////////
std::unique_lock<std::mutex> Application::getLock() {
   std::unique_lock<std::mutex> l(instance()._busy);
   return std::move(l);
}

////////////////////////////////////////////////////////////////////////////////////////
double Application::generateRandom(double low, double high) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(low, high);
    return dis(gen);
}

////////////////////////////////////////////////////////////////////////////////////////
long double Application::secondsSinceInit() {
    return std::chrono::duration<long double>(std::chrono::steady_clock::now() - instance()._startTime).count();
}
