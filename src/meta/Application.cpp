#include "Application.h"
#include "BaseWidget.h"
#include <utility>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
Application::Application()
: _debug_logger(std::cout, "debug")
, _info_logger(std::cout, "info")
, _warn_logger(std::cout, "WARNING")
, _error_logger(std::cout, "ERROR")
{
   TypeManager::instance()._registerTypes();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Window> Application::createWindow(const std::string &title, int width, int height, const std::vector<Window::Flags> &flags, int targetFPS){
   if (_window){
      return nullptr;
   }
   _window = std::shared_ptr<Window>(new Window(title, width, height, flags, targetFPS));
   //set working directory
   _workingDirectory = std::filesystem::current_path().string();
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
   //todo: should this happen immediately on addChild or should we wait until the parent is
   // added to the tree? Potentially a bunch of stuff could be added to tree and then not actually be in the tree
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
      widget->isInLayout = parent->isLayout;
      //recalculate the size rect if need to
      //todo: fix size published twice (setrect and later _publishSize
      if (widget->isAnchored() || widget->isLayout){
         //anchoring and layout of children managed by this widget
         widget->setRect(widget->_rect.value);
      }
      if (widget->isInLayout){
         //placement of layout managed by parent
         parent->setRect(parent->_rect.value);
      }
      if (!hasInit) {
         widget->_init();
         hasInit = true;
         widget->_publishSize();
      }
      widget->_on_enter_tree();
      parent->__on_child_added(widget);
      queue.pop();
   }
}
