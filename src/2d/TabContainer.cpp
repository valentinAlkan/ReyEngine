#include "TabContainer.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::render() const {
//   auto& roundness = tabTheme->roundness;
//   auto& roundnessSegments = tabTheme->roundnessSegments;
   auto& backgroundPrimary = tabTheme->background.colorPrimary;
   auto& backgroundSecondary = tabTheme->background.colorSecondary;
   auto& backgroundTertiary = tabTheme->background.colorTertiary;
   auto& lineThick = tabTheme->lineThick;

   //draw the background
   drawRectangle(_childBoundingRect, backgroundPrimary);

   //draw the border
   drawRectangleLines(_childBoundingRect, lineThick, backgroundSecondary);

   //render the tabs
   for (int i=0;i<_tabRects.size(); i++){
      auto& rect = _tabRects[i];
      drawRectangle(rect, currentTab == i ? backgroundTertiary : backgroundPrimary);
      drawRectangleLines(rect, 1.0, backgroundSecondary);
      //draw the tab child names
      auto child = _childrenOrdered.at(i);
      drawText(child->getName(), rect.pos() + Pos<int>(1,1), tabTheme->font);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::setCurrentTab(int index) {
   currentTab = index;
   cout << "Set current tab to " << index << endl;
   for (int idx=0; idx < _childrenOrdered.size(); idx++){
      auto child = _childrenOrdered[idx];
      child->setVisible(idx == currentTab.value);
      cout << child->getName() << " set visible == " << child->getVisible() << endl;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void TabContainer::arrangeChildren() {
   //make each child the same size as the tab rect
   _childBoundingRect = _rect.value.toSizeRect();
   //make it shorter, allow room for tabs
   _childBoundingRect.y += tabTheme->tabHeight.value;
   _childBoundingRect.height -= tabTheme->tabHeight.value;

   _tabRects.clear();
   int startXpos = tabTheme->tabMargin;
   for (int childIndex=0; childIndex < _childrenOrdered.size(); childIndex++){
      auto& child = _childrenOrdered[childIndex];
      child->setRect(_childBoundingRect);
      //create a tab rect for the child
      auto x = startXpos + (childIndex * tabTheme->tabWidth.value);
      auto width = tabTheme->tabWidth;
      auto height = tabTheme->tabHeight;
      Rect<int> rect(x, 0, width, height);
      _tabRects.push_back(rect);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled TabContainer::_unhandled_input(const InputEvent &event, const std::optional<UnhandledMouseInput>& mouse) {
   if (mouse && mouse.value().isInside) {
      switch (event.eventId) {
         case InputEventMouseButton::getUniqueEventId(): {
            auto mouseEvent = event.toEventType<InputEventMouseButton>();
            if (mouseEvent.isDown) return false; //only want uppies
            auto localPos = globalToLocal(mouseEvent.globalPos);
            for (int i = 0; i < _tabRects.size(); i++) {
               auto &tabRect = _tabRects[i];
               if (tabRect.isInside(localPos)) {
                  //clicked on the tab - set current index
                  setCurrentTab(i);
                  return true;
               }
            }
         }
            break;
         default:
            return false;
      }
   }
   return false;
}