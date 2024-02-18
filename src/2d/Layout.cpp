#include "Layout.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Layout::Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir)
: Control(name, typeName)
, childScales("layoutRatios")
, dir(layoutDir)
{
   isLayout = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_register_parent_properties() {
   Control::_register_parent_properties();
   Control::registerProperties();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_added(std::shared_ptr<BaseWidget> &child) {
   Application::printDebug() << child->getName() << " added to layout " << getName() << std::endl;
   arrangeChildren();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_added_immediate(std::shared_ptr<BaseWidget> &child) {
   childScales.value.push_back(1.0);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_rect_changed() {
   arrangeChildren();
}

void Layout::renderEnd() {
   //debug
   _drawRectangleLines({0, 0, _rect.value.width, _rect.value.height}, 1.0, COLORS::black);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::arrangeChildren() {
   //how much space we have to allocate
   auto totalSpace = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
   ReyEngine::Pos<int> pos;
   //how much space we will allocate to each child
   unsigned int childIndex = 0;
   auto calcRatio = [this](int startIndex) -> float{
      float sum = 0;
      for (int i=startIndex;i<childScales.size();i++){
         sum += this->childScales.get(i);
      }
      return childScales.value[startIndex] / sum;
   };

   auto sizeLeft = totalSpace;
   for (auto& child: _childrenOrdered) {
      int allowedSpace = (int)(sizeLeft * calcRatio(childIndex));
      auto actualRect = dir == LayoutDir::HORIZONTAL ? ReyEngine::Rect<int>(pos, {allowedSpace, _rect.value.height}) : ReyEngine::Rect<int>(pos, {_rect.value.width, allowedSpace});

      //enforce min/max bounds
      auto clampRect = [=](ReyEngine::Rect<int>& newRect){
         auto minWidth = child->getMinSize().x;
         auto maxWidth = child->getMaxSize().x;
         auto minHeight = child->getMinSize().y;
         auto maxHeight = child->getMaxSize().y;
         newRect.width = math_tools::clamp(minWidth, maxWidth, newRect.width);
         newRect.height = math_tools::clamp(minHeight, maxHeight, newRect.height);
      };

      clampRect(actualRect);
      auto virtualRect = actualRect; //the space the widget _would_ take up if margins didnt exist
      //apply margins
      actualRect.x += theme->layoutMargins.left();
      actualRect.y += theme->layoutMargins.top();
      actualRect.width -= (theme->layoutMargins.right() + theme->layoutMargins.left());
      actualRect.height -= (theme->layoutMargins.bottom() + theme->layoutMargins.top());

      int consumedSpace;
      switch(dir) {
         case LayoutDir::HORIZONTAL:
            pos.x += virtualRect.width;
            consumedSpace = virtualRect.width;
            break;
         case LayoutDir::VERTICAL:
            pos.y += virtualRect.height;
            consumedSpace = virtualRect.height;
            break;
      }
       std::cout << child->getName() << " rect = " << actualRect << endl;
      //apply transformations
      child->setRect(actualRect);
      childIndex++;
      //recalculate size each if we didn't use all available space
      sizeLeft -= consumedSpace;
//         childrenLeft--;
   }
}