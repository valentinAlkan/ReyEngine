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
   Tools::scope_exit exit([&](){
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
   auto isWidget = hasParent->as<Widget>();
   float parentHeight;
   float parentWidth;
   if(isWidget && getAnchoring() != Anchor::NONE) {
      parentHeight = isWidget.value()->getHeight();
      parentWidth = isWidget.value()->getWidth();
      newRect = {{0, 0}, {parentWidth, parentHeight}};
   }
   // enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   switch (_anchor) {
      case Anchor::NONE:
         newRect = r;
         break;
      case Anchor::FILL: {
         //take up as much space as parent has to offer
         break;
      }
      case Anchor::LEFT: {
         //Place on the center left of the parent
         newRect = {{0, parentHeight / 2 - getHeight() / 2}, getSize()};
         break;
      }
      case Anchor::RIGHT: {
         //Place on the center right of the parent
         newRect = {{parentWidth - getWidth(), parentHeight / 2 - getHeight() / 2}, getSize()};
         break;
      }
      case Anchor::TOP: {
         //Place at the center top of the parent
         newRect = {{parentWidth / 2 - getWidth() / 2, 0}, getSize()};
         break;
      }
      case Anchor::BOTTOM: {
         //Place at the center bottom of the parent
         newRect = {{parentWidth / 2 - getWidth() / 2, parentHeight - getHeight()}, getSize()};
         break;
      }
      case Anchor::TOP_LEFT: {
         //Place at the top left of the parent
         newRect = {{0, 0}, getSize()};
         break;
      }
      case Anchor::TOP_RIGHT: {
         //Place at the top right of the parent
         newRect = {{parentWidth - getWidth(), 0}, getSize()};
         break;
      }
      case Anchor::TOP_WIDTH: {
         //Place at the top and match the width of the parent
         newRect = {{0, 0}, {parentWidth, getHeight()}};
         break;
      }
      case Anchor::BOTTOM_RIGHT: {
         //Place at the bottom right of the parent
         newRect = {{parentWidth - getWidth(), parentHeight - getHeight()}, getSize()};
         break;
      }
      case Anchor::BOTTOM_LEFT: {
         //Place at the bottom left of the parent
         newRect = {{0, parentHeight - getHeight()}, getSize()};
         break;
      }
      case Anchor::CENTER: {
         //Place at the center of the parent
         newRect = {{parentWidth / 2 - getWidth() / 2, parentHeight / 2 - getHeight() / 2}, getSize()};
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
      throwEx(this, "setAnchoring");
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
FrameCount Widget::getEngineFrameCount() const {
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
      hasCanvas.value()->setStatus<WidgetStatus::Hover>(newValue ? this : nullptr);
   } else {
      Logger::error() << "Unable to set status on widget that is not associated with any Canvas" << endl;
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

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setFocused(bool newValue) {
   if (auto hasCanvas = getCanvas()){
      hasCanvas.value()->setStatus<WidgetStatus::Focus>(newValue ? this : nullptr);
   } else {
      Logger::error() << "Unable to set status on widget that is not associated with any Canvas" << endl;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setModal(bool newValue) {
   if (auto hasCanvas = getCanvas()){
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
