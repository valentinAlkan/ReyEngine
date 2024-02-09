#include "Workspace.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Workspace::_init() {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Workspace::_on_child_added(shared_ptr<BaseWidget>& widget) {
   //update the
   Application::printDebug() << "Added child " << widget->getName() << " to workspace!";
}


/////////////////////////////////////////////////////////////////////////////////////////
Handled Workspace::_on_drag_drop(std::shared_ptr<Draggable> draggable){
   if (draggable){
      //see which child instanceName is free
      int extension = 1;
      std::string instanceName = draggable->id;
      while(getChild(instanceName)){
         instanceName = draggable->id + "_" + to_string(extension++);
         if (extension == INT_MAX){
            throw std::runtime_error("Too many children with the same or similar instanceName as " + instanceName);
         }
      }
      auto typemeta = TypeManager::getType(draggable->id);
      if (typemeta->isVirtual) throw std::runtime_error("Cannot instantiate virtual type " + draggable->id);
      PropertyPrototypeMap empty;
      auto newWidget = TypeManager::instance().deserialize(draggable->id, instanceName, empty);
      newWidget->setInEditor(true);
      newWidget->setEditorSelected(true);
      addChild(newWidget);
      newWidget->setPos(getLocalMousePos());
      //publish widget add event
      EventWidgetAdded event(toEventPublisher());
      event.widget = newWidget;
      publish<EventWidgetAdded>(event);
      return true;
   }
   return false;
}