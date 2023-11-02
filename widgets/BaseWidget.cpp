#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include "EventManager.h"
#include <iostream>
#include <utility>

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(const std::string& name, std::string  typeName, Rect<float> rect)
:_name(std::move(name))
, _typeName(std::move(typeName))
, _rect("_rect", rect)
, _isProcessed("_isProcessed")
, _rid(Application::instance().getNewRid())
{}

BaseWidget::~BaseWidget() {
//   auto thiz = shared_from_this();
//   EventManager::unsubscribe(thiz);
};

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::setName(const std::string& newName, bool append_index) {
   auto lock = childSafetyLock();
   //if the child has a sibling by the same name, it cannot be renamed
   if (!_parent.expired()) {
      //has a parent
      auto self = toBaseWidget();
      string _newName = newName;
      auto parent = _parent.lock();
      if (parent->getChild(newName)) {
         //parent has existing child with that name
         //if we are allowed to, just append an index to the name (start at 2)
         if (append_index) {
            int index = 2;
            while (parent->getChild(newName + to_string(index))) {
               index++;
            }
            //todo: is this right?
            _newName = newName + to_string(index);
         } else {
            return false;
         }
      }
      //parent does not have a child with the name
      parent->rename(self, _newName);
   }
   //root widget, no need to deal with parent
   _name = newName;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::rename(WidgetPtr &child, const std::string &newName) {
   //rename but not move
   //find the existing reference to the child
   auto childIter = _children[child->_name];
   auto oldName = child->_name;
   _children[newName] = childIter;
   _children.erase(oldName);
   child->_name = newName;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::setIndex(unsigned int newIndex){
   if (_parent.expired()){
      //root widget, there is no index
      return false;
   }
   auto parent = _parent.lock();
   //get reference to widget in map
   auto selfMapIter = parent->_children[_name];
   auto index = selfMapIter.first;

   //delete reference to widget in ordered vector
   auto selfVectorIter = parent->_childrenOrdered.begin() + index;
   parent->_childrenOrdered.erase(selfVectorIter);

   //insert new reference in new position
   //todo: finish
   throw std::runtime_error("not finished");



}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setGlobalPos(const Vec2<double>& newPos) {
   setPos(newPos - getGlobalPos());
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::getChild(const std::string &childName) {
   auto lock = childSafetyLock();
   auto found = _children.find(childName);
   if (found == _children.end()) {
      return nullopt;
   }
   return std::optional<WidgetPtr>(found->second.second);
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
   if (widget->getParent().lock()){
      stringstream ss;
      ss << "Widget " << widget->getName() << " already has a parent! It needs to be removed from its existing parent first!";
      Application::printError() << ss.str() << endl;
      return nullopt;
   }
   Application::registerForEnterTree(widget, *this);
   widget->isInLayout = isLayout;
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
   return found->second.second;
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<int> BaseWidget::getGlobalPos() const {
   //sum up all our ancestors' positions and add our own to it
   auto offset = getPos();
   if (!_parent.expired()){ //todo: Race conditions?
      offset += _parent.lock()->getGlobalPos();
   }
   return offset;
}

/////////////////////////////////////////////////////////////////////////////////////////
GFCSDraw::Size<int> BaseWidget::getChildRectSize() const {
   GFCSDraw::Size<double> childRect;
   for (const auto& childIter : _children){
      auto totalOffset = childIter.second.second->getRect().size() + Size<double>(childIter.second.second->getPos());
      childRect.max(totalOffset);
//     childRect +=  // add position
   }
   return childRect;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChildren(GFCSDraw::Pos<double>& textureOffset) const {
   for (const auto& [name, childIter] : _children){
      childIter.second->renderChain(textureOffset);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChain(GFCSDraw::Pos<double>& parentOffset) {
   GFCSDraw::Pos<double> localOffset;
   renderBegin(localOffset);
   _renderOffset += (localOffset + parentOffset);
   render();
   renderChildren(_renderOffset);
   _renderOffset -= (localOffset + parentOffset); //subtract local offset when we are done
   renderEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseWidget::_process_unhandled_input(InputEvent& event) {
   auto passInput = [&](InputEvent& event) {
      for (auto& [name, childIter] : _children) {
         if (childIter.second->_process_unhandled_input(event)) {
            return true;
         }
      }
      return false;
   };

   switch (inputFilter){
      case InputFilter::INPUT_FILTER_PASS_AND_PROCESS:
         return passInput(event) || _unhandled_input(event);
      case InputFilter::INPUT_FILTER_PROCESS_AND_PASS:
         return _unhandled_input(event) || passInput(event);
      case InputFilter::INPUT_FILTER_IGNORE_AND_PASS:
         return passInput(event);
      case InputFilter::INPUT_FILTER_PROCESS_AND_STOP:
         return _unhandled_input(event);
      case InputFilter::INPUT_FILTER_IGNORE_AND_STOP:
         return false;
      default:
         throw std::runtime_error("INVALID INPUT FILTER STATE!");
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setProcess(bool process) {
   Application::instance().getWindow()->setProcess(process, toBaseWidget());
}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::isRoot() {
   return Application::instance().getWindow()->getRootWidget().get() == this;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawText(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, GFCSDraw::ColorRGBA color) const{
   GFCSDraw::drawText(text, pos + getGlobalPos() + _renderOffset, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawTextCentered(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, GFCSDraw::ColorRGBA color) const{
   GFCSDraw::drawTextCentered(text, pos + getGlobalPos() + _renderOffset, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangle(const Rect<int>& rect, GFCSDraw::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<int> newRect(rect + getGlobalPos() + _renderOffset);
   GFCSDraw::drawRectangle(newRect, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleLines(const GFCSDraw::Rect<int>& rect, float lineThick, GFCSDraw::ColorRGBA color) const {
   Rect<int> newRect(rect + getGlobalPos() + _renderOffset);
   GFCSDraw::drawRectangleLines(newRect, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRounded(const GFCSDraw::Rect<int> &rect, float roundness, int segments, GFCSDraw::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<double> newRect(rect + Pos<int>(getGlobalPos()) + Pos<int>(_renderOffset));
   GFCSDraw::drawRectangleRounded(newRect, roundness, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRoundedLines(const GFCSDraw::Rect<int> &rect, float roundness, int segments,float lineThick, GFCSDraw::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + Pos<double>(getGlobalPos()) + Pos<double>(_renderOffset));
   GFCSDraw::drawRectangleRoundedLines(newRect, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleGradientV(const GFCSDraw::Rect<int>& rect, GFCSDraw::ColorRGBA color1, GFCSDraw::ColorRGBA color2) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + getGlobalPos() + _renderOffset);
   GFCSDraw::drawRectangleGradientV(newRect, color1, color2);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string BaseWidget::serialize() {
   stringstream data;
   data << _name << " - " << _typeName << ":\n";
   for (const auto& [anme, property] : _properties){
      auto value = property->toString();
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
   _register_parent_properties();
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