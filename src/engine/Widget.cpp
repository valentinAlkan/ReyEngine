#include "Widget.h"
#include "Layout.h"
#include "MiscTools.h"
#include "Canvas.h"
#include "Window.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Widget::__init(){
   Internal::ReyObject::__init();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Widget::__on_added_to_tree() {
   auto parent = _node->getParent();
   while (parent){
      if (auto isWidget = parent->as<Widget>()){
         _parentWidget = isWidget.value();
         break;
      }
      parent = parent->getParent();
   }
   theme = _parentWidget && _parentWidget->theme ? _parentWidget->theme : make_shared<Theme>();
   Internal::ReyObject::__on_added_to_tree();
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<R_FLOAT> Widget::getChildBoundingBox() const {
   Rect<R_FLOAT> retval = getSizeRect();
   for (const auto& child : getChildren()){
      if (auto isWidget = child->as<Widget>()) {
         auto& _child = isWidget.value();
         retval = retval.combine(_child->getRect());
      }
   }
   return retval;
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::calculateAnchoring(const Rect<R_FLOAT>& r){

   ReyEngine::Rect<R_FLOAT> newRect = r;
   Tools::ScopeExit exit([&](){
      //enforce min/max sizes and apply rectangle (bypassing callbacks and such)
      newRect.setSize(newRect.size().max(minSize));
      newRect.setSize(newRect.size().min(maxSize));
      if (newRect != getRect()) {
         applyRect(newRect);
      }
   });

   if (!_node) return; //not in tree (yet) - allowes resizing in ctor
   auto hasParent = _node->getParent();
   if (!hasParent) return;
   auto parentWidget = hasParent->as<Widget>();
   float anchorX = 0;
   float anchorY = 0;
   float anchorHeight = 0;
   float anchorWidth = 0;
   if(parentWidget && getAnchoring() != Anchor::NONE) {
      if (_anchorArea) {
         anchorX = _anchorArea->x;
         anchorY = _anchorArea->y;
         anchorHeight = parentWidget.value()->_anchorArea->height;
         anchorWidth = parentWidget.value()->_anchorArea->width;
      } else {
         anchorHeight = parentWidget.value()->getHeight();
         anchorWidth = parentWidget.value()->getWidth();
      }
      newRect = {{anchorX, anchorY}, {anchorWidth, anchorHeight}};
   }
   // enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   auto padLeft = [&](){newRect.x += theme->widgetPadding.left(); newRect.width -= theme->widgetPadding.left();};
   auto padTop = [&](){newRect.y += theme->widgetPadding.top(); ; newRect.height -= theme->widgetPadding.top();};
   auto padRight = [&](){newRect.width -= theme->widgetPadding.right();};
   auto padBottom = [&](){newRect.height -= theme->widgetPadding.bottom();};
   auto padAll = [&](){padTop(); padBottom(); padRight(); padLeft();};
//   auto padThai;
//   auto patte;
   switch (_anchor) {
      case Anchor::NONE:
         newRect = r;
         break;
      case Anchor::FILL: {
         //take up as much space as parent has to offer
         //check padding - which is just a buffer from the outside
         if (theme) padAll();
         break;
      }
      case Anchor::LEFT: {
         //Place on the center left of the parent
         newRect = {{anchorX, anchorHeight / 2 - getHeight() / 2}, getSize()};
         if (theme) padLeft();
         break;
      }
      case Anchor::RIGHT: {
         //Place on the center right of the parent
         newRect = {{anchorWidth - getWidth(), anchorHeight / 2 - getHeight() / 2}, getSize()};
         if (theme) padRight();
         break;
      }
      case Anchor::TOP: {
         //Place at the center top of the parent
         newRect = {{anchorWidth / 2 - getWidth() / 2, anchorY}, getSize()};
         if (theme) padTop();
         break;
      }
      case Anchor::BOTTOM: {
         //Place at the center bottom of the parent
         newRect = {{anchorWidth / 2 - getWidth() / 2, anchorHeight - getHeight()}, getSize()};
         if (theme) padBottom();
         break;
      }
      case Anchor::TOP_LEFT: {
         //Place at the top left of the parent
         newRect = {{anchorWidth, anchorHeight}, getSize()};
         if (theme) {padLeft(); padTop();}
         break;
      }
      case Anchor::TOP_RIGHT: {
         //Place at the top right of the parent
         newRect = {{anchorWidth - getWidth(), anchorY}, getSize()};
         if (theme) {padRight(); padTop();}
         break;
      }
      case Anchor::TOP_WIDTH: {
         //Place at the top and match the width of the parent
         newRect = {{anchorX, anchorY}, {anchorWidth, getHeight()}};
         if (theme) {padTop(); padLeft(); padRight();}
         break;
      }
      case Anchor::BOTTOM_RIGHT: {
         //Place at the bottom right of the parent
         newRect = {{anchorWidth - getWidth(), anchorHeight - getHeight()}, getSize()};
         if (theme) {padBottom(); padRight();}
         break;
      }
      case Anchor::BOTTOM_LEFT: {
         //Place at the bottom left of the parent
         newRect = {{anchorX, anchorHeight - getHeight()}, getSize()};
         if (theme) {padBottom(); padLeft();}
         break;
      }
      case Anchor::CENTER: {
         //Place at the center of the parent
         newRect = {{anchorWidth / 2 - getWidth() / 2, anchorHeight / 2 - getHeight() / 2}, getSize()};
         //no padding, presumably its smaller than parent otherwise use fill
         break;
      }

      case Anchor::CUSTOM: {
         //delegate anchoring to someone else
         EventAnchoring event(this, getRect(), newRect, _parentWidget);
         publish(event);
         break;
      }

      default:
         break;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setAnchoring(Anchor newAnchor) {
   if (!_node){
      throwEx("setAnchoring");
   }
   auto parent = _node->getParent();
   if (!parent){
         Logger::error() << _node->getScenePath() << ": Anchoring failure : " << getName() << " has no parent!" << endl;
         return;
      }
   if (parent->as<Layout>()) {
      Logger::error() << _node->getScenePath() << ": Children of layouts cannot have anchoring!" << endl;
      return;
   }
   _anchor = newAnchor;
   setRect(getRect());
}

///////////////////////////////////////////////////////////////////////////////////////////
EngineFrameCount Widget::getEngineFrameCount() const {
   if (auto hasWindow = getWindow()){
      return hasWindow.value()->getFrameCount();
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool Widget::isHovered() const {
   if (auto hasCanvas = getCanvas()){
      return hasCanvas.value()->getStatus<WidgetStatus::Hover>() == this;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
bool Widget::isFocused() const {
   if (auto hasCanvas = getCanvas()){
      return hasCanvas.value()->getStatus<WidgetStatus::Focus>() == this;
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////////////////
bool Widget::isModal() const {
   if (auto hasCanvas = getCanvas()){
      return hasCanvas.value()->getStatus<WidgetStatus::Modal>() == this;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setHovered(bool newValue) {
   if (auto hasCanvas = getCanvas()){
      if (!newValue && !isHovered()) return;
      hasCanvas.value()->setStatus<WidgetStatus::Hover>(newValue ? this : nullptr);
   } else {
      Logger::error() << "Unable to set status on widget that is not associated with any Canvas" << endl;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setFocused(bool newValue) {
   if (auto hasCanvas = getCanvas()){
      if (!newValue && !isFocused()) return;
      hasCanvas.value()->setStatus<WidgetStatus::Focus>(newValue ? this : nullptr);
   } else {
      Logger::error() << "Unable to set status on widget that is not associated with any Canvas" << endl;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setModal(bool newValue) {
   if (auto hasCanvas = getCanvas()){
      if (!newValue && !isModal()) return;
      hasCanvas.value()->setStatus<WidgetStatus::Modal>(newValue ? this : nullptr);
   } else {
      Logger::error() << "Unable to set status on widget that is not associated with any Canvas" << endl;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> Widget::getLocalMousePos() const {
   auto hasCanvas = getCanvas();
   if (!hasCanvas) return {};
   auto globaltransform = getGlobalTransform(false).get();
   auto cameraTransform = hasCanvas.value()->getCameraTransform();
   auto mousepos = InputManager::getMousePos().get();
   return Pos<R_FLOAT>((globaltransform * cameraTransform).inverse().transform(mousepos));
}

///////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<R_FLOAT>> Widget::toCanvasSpace(const Pos<float>& p) {
   auto globaltransform = getGlobalTransform().get();
   return {Pos<R_FLOAT>(globaltransform.transform(p))};
}

///////////////////////////////////////////////////////////////////////////////////////////
WindowSpace<Pos<R_FLOAT>> Widget::toWindowSpace(const Pos<float>& p) {
   auto globaltransform = getGlobalTransform().get();
   return {Pos<R_FLOAT>(globaltransform.transform(p))};
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setAcceptsHover(bool accepts) {
   acceptsHover = accepts;
   if (isHovered() && !acceptsHover){
      setHovered(false);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::moveToForeground() {
   if (auto canvas = getCanvas()){
      getCanvas().value()->moveToForeground(this);
   } else {
      throw std::runtime_error("Widget " + getName() + " has no canvas!");
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::moveToBackground() {
   if (auto canvas = getCanvas()){
      getCanvas().value()->moveToBackground(this);
   } else {
      throw std::runtime_error("Widget " + getName() + " has no canvas!");
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Widget*> Widget::getParentWidget() const {
   auto parent = _node->getParent();
   while (parent) {
      if (auto isWidget = parent->as<Widget>()) {
         return isWidget.value();
      } else {
         parent = parent->getParent();
      }
   }
   return {};
}
