#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include "EventManager.h"
#include <iostream>
#include <utility>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(const std::string& name, std::string  typeName, Rect<float> rect)
: _name(std::move(name))
, _typeName(std::move(typeName))
, _rect("_rect", rect)
, _isProcessed("_isProcessed")
, _anchor("Anchor", Anchor::NONE)
, _rid(Application::instance().getNewRid())
{}

BaseWidget::~BaseWidget() {
//   auto thiz = shared_from_this();
//   EventManager::unsubscribe(thiz);
};

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::setName(const std::string& newName, bool append_index) {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
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
   //root widget or orphaned, no need to deal with parent
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
void BaseWidget::setGlobalPos(const Vec2<int>& newPos) {
   setPos(newPos - getGlobalPos());
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> BaseWidget::getClampedSize(ReyEngine::Size<int> size){
   auto newX = ReyEngine::Vec2<int>(minSize.x, maxSize.x).clamp(size.x);
   auto newY = ReyEngine::Vec2<int>(minSize.y, maxSize.y).clamp(size.y);
   return {newX, newY};
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> BaseWidget::getClampedSize(){
   return getClampedSize(getSize());
}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::hasChild(const std::string &name) {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   return _children.find(name) != _children.end();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::getChild(const std::string &childName) {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   auto found = _children.find(childName);
   if (found == _children.end()) {
      return nullopt;
   }
   return {found->second.second};
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::addChild(WidgetPtr widget){
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
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
   //call immediate callback
   _on_child_added_immediate(widget);
   auto me = toBaseWidget();
   Application::printDebug() << "Registering child " << widget->getName() << " to parent " << getName() << endl;
   Application::registerForEnterTree(widget, me);
   widget->isInLayout = isLayout;
   return widget;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<BaseWidget::WidgetPtr> BaseWidget::removeChild(const std::string& name, bool quiet) {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   auto found = _children.find(name);
   if (found == _children.end()){
      if (!quiet) {
         stringstream ss;
         ss << "Widget " << getName() << " does not have a child with name <" << name << ">";
         Application::printError() << ss.str() << endl;
      }
      return nullopt;
   }
   auto child = found->second.second;
   _children.erase(found);
   child->isInLayout = false;
   return child;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::removeAllChildren() {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   for (auto& child : _children){
      child.second.second->isInLayout = false;
   }
   _children.clear();
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
Pos<int> BaseWidget::globalToLocal(const Pos<int>& global) const {
   auto globalPos = getGlobalPos();
   auto retval = global - globalPos;
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Pos<int> BaseWidget::localToGlobal(const ReyEngine::Pos<int> &local) const {
   return local + getGlobalPos();
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> BaseWidget::getChildBoundingBox() const {
   ReyEngine::Size<double> childRect;
   for (const auto& childIter : _children){
      auto totalOffset = childIter.second.second->getRect().size() + Size<double>(childIter.second.second->getPos());
      childRect.max(totalOffset);
   }
   return childRect;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChain(ReyEngine::Pos<double>& parentOffset) {
   if (!_visible) return;
   ReyEngine::Pos<double> localOffset;
   renderBegin(localOffset);
   _renderOffset += (localOffset + parentOffset);
   render();
   //renderChildren
   for (const auto& [name, childIter] : _children){
      childIter.second->renderChain(_renderOffset);
   }
   _renderOffset -= (localOffset + parentOffset); //subtract local offset when we are done
   renderEnd();
   renderEditorFeatures();
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

   if (_isEditorWidget){
      if (_process_unhandled_editor_input(event) > 0) return true;
   }

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
Handled BaseWidget::_process_unhandled_editor_input(InputEvent& event) {
   if (!_editor_selected) return false;
   switch(event.eventId){
      case InputEventMouseButton::getUniqueEventId():
      case InputEventMouseMotion::getUniqueEventId():
         auto mouseEvent = event.toEventType<InputEventMouseButton>();
         auto localPos = globalToLocal(mouseEvent.globalPos);
         switch (event.eventId){
            case InputEventMouseButton::getUniqueEventId(): {
               auto mouseButton = event.toEventType<InputEventMouseButton>();
               if (_editor_grab_handles_dragging == -1 && mouseButton.isDown) {
                  //check if we click in the grab handles
                  if (_getGrabHandle(0).isInside(localPos)) {
                     _editor_grab_handles_dragging = 0;
                     return true;
                  }
                  if (_getGrabHandle(1).isInside(localPos)) {
                     _editor_grab_handles_dragging = 1;
                     return true;
                  }
                  if (_getGrabHandle(2).isInside(localPos)) {
                     _editor_grab_handles_dragging = 2;
                     return true;
                  }
                  if (_getGrabHandle(3).isInside(localPos)) {
                     _editor_grab_handles_dragging = 3;
                     return true;
                  }
               } else if (!mouseButton.isDown && _editor_grab_handles_dragging != -1){
                  _editor_grab_handles_dragging = -1;
                  return true;
               }
               break;
            }
            case InputEventMouseMotion::getUniqueEventId():
               auto delta = event.toEventType<InputEventMouseMotion>().mouseDelta;
               //see which handle we are dragging
               auto _newRect = getRect();
               switch(_editor_grab_handles_dragging){
                  case 0:
                     //resize the rect
                     _newRect.x += delta.x;
                     _newRect.y += delta.y;
                     _newRect.width -= delta.x;
                     _newRect.height -= delta.y;
                     setRect(_newRect);
                     return true;
                  case 1:
                     _newRect.y += delta.y;
                     _newRect.height -= delta.y;
                     _newRect.width += delta.x;
                     setRect(_newRect);
                     return true;
                  case 2:
                     _newRect.height += delta.y;
                     _newRect.width += delta.x;
                     setRect(_newRect);
                     return true;
                  case 3:
                     _newRect.x += delta.x;
                     _newRect.width -= delta.x;
                     _newRect.height += delta.y;
                     setRect(_newRect);
                     return true;
                  default:
                     break;
               }
         }
   }
   return false;
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
void BaseWidget::_drawText(const std::string &text, const ReyEngine::Pos<int> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawText(text, pos + getGlobalPos() + _renderOffset, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawTextCentered(const std::string &text, const ReyEngine::Pos<int> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawTextCentered(text, pos + getGlobalPos() + _renderOffset, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangle(const Rect<int>& rect, ReyEngine::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<int> newRect(rect + getGlobalPos() + _renderOffset);
   ReyEngine::drawRectangle(newRect, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleLines(const ReyEngine::Rect<int>& rect, float lineThick, ReyEngine::ColorRGBA color) const {
   Rect<int> newRect(rect + getGlobalPos() + _renderOffset);
   ReyEngine::drawRectangleLines(newRect, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRounded(const ReyEngine::Rect<int> &rect, float roundness, int segments, ReyEngine::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<double> newRect(rect + Pos<int>(getGlobalPos()) + Pos<int>(_renderOffset));
   ReyEngine::drawRectangleRounded(newRect, roundness, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleRoundedLines(const ReyEngine::Rect<int> &rect, float roundness, int segments,float lineThick, ReyEngine::ColorRGBA color) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + Pos<double>(getGlobalPos()) + Pos<double>(_renderOffset));
   ReyEngine::drawRectangleRoundedLines(newRect, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawRectangleGradientV(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA color1, ReyEngine::ColorRGBA color2) const {
   //use the size of the param rect but use the position of our rect + the param rect
   Rect<float> newRect(rect + getGlobalPos() + _renderOffset);
   ReyEngine::drawRectangleGradientV(newRect, color1, color2);
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
   registerProperty(_anchor);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderEditorFeatures(){
   if (_editor_selected){
      //render the grab handles
      static constexpr ReyEngine::ColorRGBA GRAB_HANDLE_COLOR = COLORS::blue;
      _drawRectangleRounded(_getGrabHandle(0), 1.0, 5, GRAB_HANDLE_COLOR);
      _drawRectangleRounded(_getGrabHandle(1), 1.0, 5, GRAB_HANDLE_COLOR);
      _drawRectangleRounded(_getGrabHandle(2), 1.0, 5, GRAB_HANDLE_COLOR);
      _drawRectangleRounded(_getGrabHandle(3), 1.0, 5, GRAB_HANDLE_COLOR);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<int> BaseWidget::_getGrabHandle(int index) {
   switch(index){
      case 0:
         return {-GRAB_HANDLE_SIZE,-GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 1:
         return {getWidth(),-GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 2:
         return {getWidth(),getHeight(),GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 3:
         return {-GRAB_HANDLE_SIZE, getHeight(),GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
       default:
           throw std::runtime_error("Invalid grab handle index");
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> BaseWidget::getWidgetAt(ReyEngine::Pos<int> pos) {
   //query the widget and figure out which of its children is the topmost widget at the position
   std::shared_ptr<BaseWidget> widgetAt;
   std::function<std::shared_ptr<BaseWidget>(std::shared_ptr<BaseWidget>)> findWidget = [&](std::shared_ptr<BaseWidget> currentWidget){
      if (currentWidget->getGlobalRect().isInside(pos)) widgetAt = currentWidget;
      for (auto& child : currentWidget->getChildren()) {
         if (child->getGlobalRect().isInside(pos)) widgetAt = child;
         findWidget(child);
      }
      return std::shared_ptr<BaseWidget>();
   };
   findWidget(toBaseWidget());
   if (widgetAt) return widgetAt;
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<int> BaseWidget::calculateAnchoring(const ReyEngine::Rect<int>& r) {
    //todo: not implemented
    return {};
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setRect(const ReyEngine::Rect<int>& r){
   ReyEngine::Rect<int> newRect(r);
   auto parent = getParent();
   if(getAnchoring() != Anchor::NONE) {
      if (!parent.expired()) {
         newRect = {{0, 0}, getSize()};
      } else if (isRoot()) {
         newRect = {{0, 0}, Application::instance().getWindow()->getSize()};
      } else {
         _rect.set(r);
         _on_rect_changed();
         _publishSize();
         return;
         //todo: this is weird, find a way to make this flow better
      }
   }
   switch (_anchor.value) {
      case Anchor::NONE:
         _rect.set(r);
         break;
      case Anchor::FILL: {
         //take up as much space as parent has to offer
         _rect.set(newRect);
         break;
//         case Anchor::BOTTOM:
//            //bottom of this widget aligns with bottom of parent
//            _rect.set()
      }
      default:
         break;
   }
   _on_rect_changed();
   _publishSize();
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setPos(int x, int y){
   ReyEngine::Rect<int> r(x, y, _rect.value.width, _rect.value.height);
   setRect(r);
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setPos(const ReyEngine::Pos<int>& pos) {
   ReyEngine::Rect<int> r(pos, _rect.value.size());
   setRect(r);
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setSize(const ReyEngine::Size<int>& size){
   ReyEngine::Rect<int> r(_rect.value.pos(), size);
   setRect(r);
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setWidth(int width){
   ReyEngine::Rect<int> r(_rect.value.pos(), {width, _rect.value.height});
   setRect(r);
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setHeight(int height){
   ReyEngine::Rect<int> r(_rect.value.pos(), {_rect.value.width, height});
   setRect(r);
}
