#include "Application.h"
#include "BaseWidget.h"
#include <utility>

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Window> Application::createWindow(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS){
   if (_window){
      return nullptr;
   }
   _window = std::shared_ptr<Window>(new Window("MainWindow", width, height, flags, targetFPS));
   return _window;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::registerForEnterTree(std::shared_ptr<BaseWidget>& widget, std::shared_ptr<BaseWidget>& parent) {
   instance()._initQueue.emplace(widget, parent);
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

////////////////////////////////////////////////////////////////////////////////////////
void Application::processEnterTree() {
   //todo: also process enter tree
   auto& queue = instance()._initQueue;
   while (!queue.empty()){
      auto& p = queue.front();
      auto& widget = p.first;
      auto& parent = p.second;
      if (parent->hasChild(widget->getName())){
         throw std::runtime_error("Parent "  + parent->getName() + " already has child with the name " + widget->getName());
      }
      auto& hasInit = widget->_has_inited;
      auto newIndex = parent->_childrenOrdered.size(); //index of new child's location in ordered vector
      parent->_children[widget->getName()] = std::pair<int, std::shared_ptr<BaseWidget>>(newIndex, widget);
      parent->_childrenOrdered.push_back(widget);
      widget->_parent = parent->toBaseWidget();
      if (!hasInit) {
         widget->_init();
         hasInit = true;
         widget->_on_rect_changed(); //initialize the rectangle
         widget->_publishSize();

      }
      widget->_on_enter_tree();
      parent->_on_child_added(widget);
      queue.pop();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::clearHover() {
   if (!instance()._hovered.expired()) {
      auto oldHover = instance()._hovered.lock();
      oldHover->_hovered = false;
      oldHover->_on_mouse_exit();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Application::setHover(std::weak_ptr<BaseWidget> widget) {
   instance().clearHover();
   if (!widget.expired()){
      instance()._hovered = widget;
      widget.lock()->_hovered = true;
      widget.lock()->_on_mouse_enter();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::weak_ptr<BaseWidget>> Application::getHovered() {
   if (instance()._hovered.expired()) return nullopt;
   return instance()._hovered;
}