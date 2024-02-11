#include "Layout.h"

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
   _drawRectangleLines({0, 0, _rect.value.width, _rect.value.height}, 2.0, COLORS::black);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::arrangeChildren() {
   //how much space we have to allocate
   auto totalSpace = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
   auto pos = ReyEngine::Pos<int>(0, 0);
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
      auto newRect = dir == LayoutDir::HORIZONTAL ? ReyEngine::Rect<int>(pos, {allowedSpace, _rect.value.height}) : ReyEngine::Rect<int>(pos, {_rect.value.width, allowedSpace});
      auto minSize = dir == LayoutDir::HORIZONTAL ? child->getMinSize().x : child->getMinSize().y;
      auto maxSize = dir == LayoutDir::HORIZONTAL ? child->getMaxSize().x : child->getMaxSize().y;
      //enforce min/max bounds
      int consumedSpace;
      switch(dir) {
         case LayoutDir::HORIZONTAL:
            newRect.width = math_tools::clamp(minSize, maxSize, newRect.width);
            pos.x += newRect.width;
            consumedSpace = newRect.width;
            break;
         case LayoutDir::VERTICAL:
            newRect.height = math_tools::clamp(minSize, maxSize, newRect.height);
            pos.y += newRect.height;
            consumedSpace = newRect.height;
            break;
      }
      //apply transformations
      child->setRect(newRect);
      childIndex++;
      //recalculate size each if we didn't use all available space
      sizeLeft -= consumedSpace;
//         childrenLeft--;
   }
}