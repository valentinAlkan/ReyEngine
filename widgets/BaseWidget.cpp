#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include <iostream>

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(std::string name, std::string typeName)
:_name(std::move(name))
, _typeName(typeName)
, _rid(Application::instance().getNewRid())
{}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::setName(const std::string& newName, bool append_index) {
   auto lock = childSafetyLock();
   //if the child has a sibling by the same name, it cannot be renamed
   if (_parent) {
      //has a parent
      auto self = shared_from_this();
      string _newName;
      auto parent = _parent.value();
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
   auto parent = _parent;
   while (parent && parent.value()) {
      offset.x += parent.value()->getPos().x;
      offset.y += parent.value()->getPos().y;
      parent = parent.value()->_parent;
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
      data << PropertyMeta::SEP << property->typeName();
      data << PropertyMeta::SEP << value.size();
      data << PropertyMeta ::SEP << value;
      data << ";";
   }
   return data.str();
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::deserialize(std::map<std::string, std::string> propertyData){
   //register all properties so we know what's what
   registerProperties();
   //move the properties to their new home
   for (auto& [name, data] : propertyData){
      _properties[name]->load(data);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::registerProperty(BaseProperty& property) {
   auto found = _properties.find(property.instanceName());
   if (found != _properties.end()){
      throw std::runtime_error("Property name " + property.instanceName() + " already exists!");
   }
   _properties[property.instanceName()] = &property;
}