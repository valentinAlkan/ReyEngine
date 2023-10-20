#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include "EventManager.h"
#include <iostream>

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(const std::string& name, const std::string& typeName, Rect<float> rect)
:_name(std::move(name))
, _typeName(typeName)
, _rect("_rect", rect)
, _isProcessed("_isProcessed")
, _rid(Application::instance().getNewRid())
{
}

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
   widget->_parent = toBaseWidget();
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
void BaseWidget::renderChildren(GFCSDraw::Vec2<float>& textureOffset) const {
   for (const auto& [name, child] : _children){
      child->renderChain(textureOffset);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChain(GFCSDraw::Vec2<float>& parentOffset) {
   GFCSDraw::Vec2<float> localOffset;
   renderBegin(localOffset);
   _textureRenderModeOffset = localOffset + parentOffset;
   renderChildren(_textureRenderModeOffset);
   render();
   _textureRenderModeOffset = parentOffset; //subtract local offset when we are done
   renderEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseWidget::_process_unhandled_input(InputEvent& event) {
   auto passInput = [&](InputEvent& event) {
      for (auto &[name, child]: getChildren()) {
         if (child->_process_unhandled_input(event)) {
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
void BaseWidget::_drawText(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, Color color) const{
   GFCSDraw::drawText(text, pos + getGlobalPos() + _textureRenderModeOffset, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawTextCentered(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, Color color) const{
   GFCSDraw::drawTextCentered(text, pos + getGlobalPos() + _textureRenderModeOffset, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangle(const Rect<int>& rect, Color color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<int> newRect(rect + getGlobalPos() + _textureRenderModeOffset);
   GFCSDraw::drawRectangle(newRect, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRounded(const GFCSDraw::Rect<int> &rect, float roundness, int segments, Color color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<int> newRect(rect + getGlobalPos() + _textureRenderModeOffset);
   GFCSDraw::drawRectangleRounded(newRect, roundness, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRoundedLines(const GFCSDraw::Rect<float> &rect, float roundness, int segments,float lineThick, Color color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + getGlobalPos() + _textureRenderModeOffset);
   GFCSDraw::drawRectangleRoundedLines(newRect, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleGradientV(const GFCSDraw::Rect<int>& rect, Color color1, Color color2) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + getGlobalPos() +_textureRenderModeOffset);
   GFCSDraw::drawRectangleGradientV(newRect, color1, color2);
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