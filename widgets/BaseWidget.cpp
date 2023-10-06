#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include "EventManager.h"
#include <iostream>

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(std::string name, std::string typeName)
:_name(std::move(name))
, _typeName(typeName)
, _rect("_rect")
, _isProcessed("_isProcessed")
, _rid(Application::instance().getNewRid())
{}

BaseWidget::~BaseWidget() {
   auto thiz = shared_from_this();
   EventManager::unsubscribe(thiz);
};

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::setName(const std::string& newName, bool append_index) {
   auto lock = childSafetyLock();
   //if the child has a sibling by the same name, it cannot be renamed
   if (!_parent.expired()) {
      //has a parent
      auto self = shared_from_this();
      string _newName;
      auto parent = _parent.lock();
      if (parent->getChild(newName)) {
         //parent has existing child with that name
         //if we are allowed to, just append an index to the name (start at 2)
         if (append_index) {
            int index = 2;
            while (parent->getChild(newName + to_string(index))) {
               index++;
            }
            _newName = newName + to_string(index);
            //remove the child
            parent->removeChild(self);
         } else {
            return false;
         }
      }
      //parent does not have a child with the name
      self->_name = newName;
      //add the child back to the parent
      parent->addChild(self);
   }
   //root widget, no need to deal with parent
   _name = newName;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::getChild(const std::string &childName) {
   auto lock = childSafetyLock();
   auto found = _children.find(childName);
   if (found == _children.end()) {
      return nullopt;
   }
   return std::optional<WidgetPtr>(found->second);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::addChild(WidgetPtr widget){
   auto lock = childSafetyLock();
   auto found = getChild(widget->getName());
   if (found){
      stringstream ss;
      ss << "Widget " << getName() << " already has a child with name <" << widget->getName() << ">";
      Application::printError() << ss.str() << endl;
      return nullopt;
   }
   _children[widget->getName()] = widget;
   widget->_parent = shared_from_this();
   return widget;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::removeChild(WidgetPtr widget) {
   auto lock = childSafetyLock();
   auto found = _children.find(widget->getName());
   if (found == _children.end()){
      stringstream ss;
      ss << "Widget " << getName() << " does not have a child with name <" << widget->getName() << ">";
      Application::printError() << ss.str() << endl;
      return nullopt;
   }
   _children.erase(found);
   return found->second;
}

/////////////////////////////////////////////////////////////////////////////////////////
Vec2<double> BaseWidget::getGlobalPos() const {
   //sum up all our ancestors' positions and add our own to it
   auto offset = getPos();
   if (!_parent.expired()){ //todo: Race conditions?
      offset += _parent.lock()->getGlobalPos();
   }
   return offset;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChildren() {
   for (const auto& [name, child] : _children){
      child->renderChildren();
      child->render();
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setProcess(bool process) {
   Application::instance().getWindow()->setProcess(process, shared_from_this());
}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::isRoot() {
   return Application::instance().getWindow()->getRootWidget().get() == this;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawText(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, Color color) const{
   GFCSDraw::drawText(text, getGlobalPos(), fontSize, color);
}



/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::operator==(const shared_ptr<BaseWidget> &other) const {
   return _rid == other->_rid;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string BaseWidget::serialize() {
   stringstream data;
   data << _name << " - " << _typeName << ":\n";
   for (const auto& [anme, property] : _properties){
      auto value = property->dump();
      data << PropertyMeta::INDENT << property->instanceName();
      data << PropertyMeta::SEP << property->typeName(); //todo: ? pretty sure this isn't actually necesary
      data << PropertyMeta::SEP << value.size();
      data << PropertyMeta ::SEP << value;
      data << ";";
   }
   return data.str();
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_deserialize(PropertyPrototypeMap& propertyData){
   //register all properties so we know what's what
   registerProperties();
   //   move the properties to their new home
   for (auto& [name, data] : propertyData){
      auto found = _properties.find(name);
      if (found == _properties.end()){
         throw std::runtime_error("Property " + name + " of type " + data.typeName + " not registered to type " + _typeName + ". Did you remember to call ParentType::registerProperties() for each parent type?");
      }
      found->second->_load(data);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::registerProperties() {
   registerProperty(_rect);
   registerProperty(_isProcessed);
}