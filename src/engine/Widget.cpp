#include "Widget.h"
#include "Layout.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;

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

      default:
         break;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Widget::setAnchoring(Anchor newAnchor) {
   if (_node->getParent()->as<Layout>()){
      Logger::error() << _node->getScenePath() << ": Children of layouts cannot have anchoring!";
      return;
   }
   _anchor = newAnchor;
   if (!_node->getParent()) return;
   if (auto hasParent = _node->getParent()->as<Widget>()) {
      setRect(getRect());
   }
}