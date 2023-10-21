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
void Application::registerForEnterTree(std::shared_ptr<BaseWidget>& widget, BaseWidget& parent) {
   instance()._initTreeList.emplace_back(widget, parent);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForApplicationReady(std::shared_ptr<BaseWidget>& widget) {
   if (!isReady()) {
      instance()._applicationReadyList.insert(widget);
   } else {
      widget->_on_application_ready();
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
   for (auto& widget : instance()._applicationReadyList){
      widget->_on_application_ready();
   }
   instance()._applicationReadyList.clear(); //done with this forever
   for (auto& cb : instance()._initListArbCallback){
      cb();
   }
   instance()._initListArbCallback.clear(); //done with this forever
   instance()._is_ready = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::processEnterTree() {
   //todo: also process enter tree
   auto& initTreeList = instance()._initTreeList;
   for (auto& [widget, parent] : initTreeList){
      auto& hasEnteredTreeBefore = widget->_has_entered_tree_before;
      if (!hasEnteredTreeBefore) {
         widget->_init();
         widget->_has_entered_tree_before = true;
      }
      parent._children[widget->getName()] = widget;
      widget->_parent = parent.toBaseWidget();
   }
   initTreeList.clear();
}