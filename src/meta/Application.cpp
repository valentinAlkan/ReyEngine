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
   TypeManager::instance()._registerTypes();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Window> Application::createWindow(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS){
   _windows.push_back(std::shared_ptr<Window>(new Window(title, width, height, flags, targetFPS)));
   _windows.back()->getCanvas()->setRect({}); //will auto fill

   return _windows.back();
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