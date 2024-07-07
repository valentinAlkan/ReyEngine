#include "Layout.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Layout::Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir)
: BaseWidget(name, typeName)
, childScales("layoutRatios")
, dir(layoutDir)
{
   isLayout = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_register_parent_properties() {
   BaseWidget::_register_parent_properties();
   BaseWidget::registerProperties();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_added(std::shared_ptr<BaseWidget> &child) {
   Logger::debug() << child->getName() << " added to layout " << getName() << std::endl;
   if (childScales.size() < getChildren().size()){
      childScales.append(1.0);
   }
   arrangeChildren();
}

/////////////////////////////////////////////////////////////////////////////////////////
//void Layout::_on_child_added_immediate(std::shared_ptr <BaseWidget> &child) {
//   //ensure we have the correct number of values for the amount of children we have
//   rectifyScales();
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_removed(std::shared_ptr<BaseWidget>& child){
   if (childScales.size() > getChildren().size()){
      childScales.pop_back();
   }
}

void Layout::renderEnd() {
   //debug
//   drawRectangleLines({0, 0, _rect.value.width, _rect.value.height}, 1.0, COLORS::black);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::arrangeChildren() {
   if (dir == LayoutDir::GRID){
         //divide the space into boxes, each box being large enough to exactly contain the largest child (in either dimension)
         // Center each child inside it's respective box.

         //determine box size
         Size<int> boundingBox;
         for (const auto& child : getChildren()){
            boundingBox = boundingBox.max(child->getMaxSize());
         }
         if (!boundingBox.x || !boundingBox.y) return; //invalid rect
         //create subrects to lay out the children
         if (_rect.value.size().x && _rect.value.size().y) {
            for (int i = 0; i < getChildren().size(); i++) {
               auto &child = getChildren().at(i);
               auto subrect = _rect.value.toSizeRect().getSubRect(boundingBox, i);
               child->setRect(subrect);
            }
         }
      } else {
      //how much space we have to allocate
      auto totalSpace = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
      ReyEngine::Pos<int> pos;
      //how much space we will allocate to each child
      unsigned int childIndex = 0;
      auto calcRatio = [this](int startIndex) -> float {
          float sum = 0;
          for (int i = startIndex; i < childScales.size(); i++) {
             sum += this->childScales.get(i);
          }
          if (!childScales.size()) childScales.append(1.0); //always ensure we have a child scale
          return childScales.value[startIndex] / sum;
      };

      auto sizeLeft = totalSpace;
      for (auto &child: getChildren()) {
         int allowedSpace = (int) (sizeLeft * calcRatio(childIndex));
         auto actualRect = dir == LayoutDir::HORIZONTAL ? ReyEngine::Rect<int>(pos, {allowedSpace, _rect.value.height})
                                                        : ReyEngine::Rect<int>(pos, {_rect.value.width, allowedSpace});

         //enforce min/max bounds
         auto clampRect = [=](ReyEngine::Rect<int> &newRect) {
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
         switch (dir) {
            case LayoutDir::HORIZONTAL:
               pos.x += virtualRect.width;
                 consumedSpace = virtualRect.width;
                 break;
            case LayoutDir::VERTICAL:
               pos.y += virtualRect.height;
                 consumedSpace = virtualRect.height;
                 break;
         }
         child->setRect(actualRect);
         childIndex++;
         //recalculate size each if we didn't use all available space
         sizeLeft -= consumedSpace;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> Layout::calculateIdealBoundingBox() {
   Size<int> idealBoundingBox;
   if (dir == LayoutDir::GRID){
      throw std::runtime_error("not implemented!");
//      //divide the space into boxes, each box being large enough to exactly contain the largest child (in either dimension)
//      // Center each child inside it's respective box.
//
//      //determine box size
//      Size<int> boundingBox;
//      for (const auto& child : getChildren()){
//         boundingBox.max(child->getMaxSize());
//      }
//      //create subrects to lay out the children
//      if (_rect.value.size().x && _rect.value.size().y) {
//         for (int i = 0; i < getChildren().size(); i++) {
//            auto &child = getChildren().at(i);
//            auto subrect = _rect.value.toSizeRect().getSubRect(boundingBox, i);
//            child->setRect(subrect);
//         }
//      }
   } else {
      if (dir == LayoutDir::VERTICAL){
         for (auto& child : getChildren()){
            auto childMinSize = child->getMinSize().max({0, 0});
            idealBoundingBox.x = Math::max(idealBoundingBox.x, childMinSize.x);
            idealBoundingBox.y += childMinSize.y;
         }
      } else {
         for (auto& child : getChildren()){
            auto childMinSize = child->getMinSize().min({0,0});
            idealBoundingBox.x += childMinSize.x;
            idealBoundingBox.y = Math::max(idealBoundingBox.y, childMinSize.y);
         }
      }
   }
   return idealBoundingBox;
}