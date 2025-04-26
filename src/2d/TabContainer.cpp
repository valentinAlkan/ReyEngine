#include "TabContainer.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::render2D() const {
//   auto& roundness = theme->roundness;
//   auto& roundnessSegments = theme->roundnessSegments;
   auto& backgroundPrimary = theme->background.colorPrimary;
   auto& backgroundSecondary = theme->background.colorSecondary;
   auto& backgroundTertiary = theme->background.colorTertiary;
   auto& lineThick = theme->outline.linethick;

   //draw the background
   drawRectangle(getRect(), backgroundPrimary);

   //draw the border
   drawRectangleLines(_childBoundingRect, lineThick, backgroundSecondary);

   //render the tabs
   if (!currentTab) return;
   for (auto& tab : _tabs){
      auto& rect = tab.tabRect;
      drawRectangle(rect, currentTab.value() == tab.widget ? backgroundTertiary : backgroundPrimary);
      //drawLines
      drawLine(rect.leftSide(), 1.0, backgroundSecondary);
      drawLine(rect.top(), 1.0, backgroundSecondary);
      drawLine(rect.rightSide(), 1.0, backgroundSecondary);
//      drawRectangleLines(rect, 1.0, backgroundSecondary);
      //draw the tab child names
      drawText(tab.widget->getName(), rect.pos() + Pos<int>(1,1), theme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::_on_child_added_to_tree(TypeNode* node) {
   auto isWidget = node->as<Widget>();
   if (!isWidget) return;

   // if we already have a tab widget, immediately hide any subsequent children
   auto& child = isWidget.value();
   if (currentTab){
      child->setVisible(false);
   } else {
      setCurrentTab(child);
   }
   arrangeChildren();
}

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::setCurrentTab(Widget* w) {
   currentTab = w;
   //if current tab is null
   if (!currentTab){
      currentTab = {};
      Logger::debug() << "Clearing tab container current widget" << endl;
      return;
   }

   auto children = getChildrenAs<Widget>();
   for (auto child : children){
      child->setVisible(child == currentTab.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::arrangeChildren() {
   //make each child the same size as the tab rect
   _childBoundingRect= getRect().toSizeRect();
   //make it shorter, allow room for tabs
   _childBoundingRect.y += tabHeight;
   _childBoundingRect.height -= tabHeight;

   _tabs.clear();
   R_FLOAT x = 0;
   for (auto child : getChildrenAs<Widget>()){
      layoutApplyRect(child, _childBoundingRect);
      //create a tab rect for the child
      auto width = measureText(child->getName(), getTheme().font).x + tabPadding;
      auto height = tabHeight;
      Rect<R_FLOAT> rect(x, 0, width, height);
      x += width;
      _tabs.emplace_back(child, rect);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* TabContainer::_unhandled_input(const InputEvent& event){
   if (auto isMouse = event.isMouse()) {
      switch (event.eventId) {
         case InputEventMouseButton::getUniqueEventId(): {
            auto mouseEvent = event.toEvent<InputEventMouseButton>();
            if (mouseEvent.isDown) return nullptr; //only want uppies
            auto localPos = isMouse.value()->getLocalPos();
            for (auto& tabData : _tabs){
               if (tabData.tabRect.contains(localPos)) {
                  //clicked on the tab - set current index
                  setCurrentTab(tabData.widget);
                  return this;
               }
            }
         }
            break;
         default:
            return nullptr;
      }
   }
   return nullptr;
}