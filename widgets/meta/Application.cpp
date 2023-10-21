#include "Application.h"
#include "BaseWidget.h"
#include <utility>

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Window>> Application::createWindow(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS){
   if (_window){
      return nullptr;
   }
   _window = std::shared_ptr<Window>(new Window("MainWindow", width, height, flags, targetFPS));
   return _window;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForInit(std::shared_ptr<BaseWidget> widget) {
   if (!isReady()) {
      instance()._initListWidget.insert(widget);
   } else {
      widget->_on_application_ready();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForInit(std::function<void()> cb) {
   if (!isReady()) {
      instance()._initListArbCallback.push_back(cb);
   } else {
      cb();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::ready() {
   for (auto& widget : instance()._initListWidget){
      widget->_on_application_ready();
   }
   instance()._initListWidget.clear(); //done with this forever
   for (auto& cb : instance()._initListArbCallback){
      cb();
   }
   instance()._initListArbCallback.clear(); //done with this forever
   instance()._is_ready = true;
}