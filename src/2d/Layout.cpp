#include "Layout.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;

static constexpr bool VERBOSE = true;
/// A struct that helps us layout widgets. Applies changes on dtor.
struct Layout::LayoutHelper {
   LayoutHelper(LayoutDir layoutDir, int index, std::shared_ptr<BaseWidget>& child)
   : childIndex(index)
   , layoutDir(layoutDir)
   , child(child)
   {}
   ~LayoutHelper(){
      //apply the rect
      if constexpr (VERBOSE) Logger::debug() << "Parent " << child->getParent().lock()->getName() << " applying rectangle " << pendingRect << " to child " << child->getName() << endl;

      //apply margins - this should always be the very last thing we do
      auto theme = child->getParent().lock()->getTheme();
      pendingRect.x += theme->layoutMargins.left();
      pendingRect.y += theme->layoutMargins.top();
      pendingRect.width -= (theme->layoutMargins.right() + theme->layoutMargins.left());
      pendingRect.height -= (theme->layoutMargins.bottom() + theme->layoutMargins.top());
      child->setRect(pendingRect);
   }
   /// Accounts for min/max
   std::optional<R_FLOAT> setPendingRect(const Rect<int>& newRect){
      Tools::AnonymousDtor dtor([&](){if constexpr (VERBOSE) Logger::debug() << "Child " << child->getName() << " will be allowed " << pendingRect.size() << " space" << endl;});
      pendingRect = newRect;
      auto maxSize = layoutDir == LayoutDir::HORIZONTAL ? child->getMaxSize().x : child->getMaxSize().y;
      auto minSize = layoutDir == LayoutDir::HORIZONTAL ? child->getMinSize().x : child->getMinSize().y;
      auto& releventDimension = layoutDir == LayoutDir::HORIZONTAL ? pendingRect.width : pendingRect.height;

      bool isConstrainedMax = releventDimension > maxSize;
      bool isConstrainedMin = releventDimension < minSize;
      if (isConstrainedMax) {
         releventDimension = maxSize;
         return maxSize;
      }
      if (isConstrainedMin) {
         releventDimension = minSize;
         return minSize;
      }
      return nullopt;
   }
   /// Sets the x or y value appropriately based on the layout - one value should always be 0
   void setReleventPosition(R_FLOAT pos) {
      switch (layoutDir) {
         case LayoutDir::HORIZONTAL:
            pendingRect.x = pos;
            break;
         case LayoutDir::VERTICAL:
            pendingRect.y = pos;
            break;
      }
   }
   /// returns height for vertical layouts and width for horizontal layouts
   int getReleventDimension(){return layoutDir == LayoutDir::VERTICAL ? pendingRect.height : pendingRect.width;}
   bool operator==(const LayoutHelper& rhs) const {return child == rhs.child;}
   const int childIndex;
   const LayoutDir layoutDir;
   std::shared_ptr<BaseWidget>& child;
private:
   Rect<R_FLOAT> pendingRect;
};

/////////////////////////////////////////////////////////////////////////////////////////
Layout::Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir)
: BaseWidget(name, typeName)
, NamedInstance(name, typeName)
, Component(name, typeName)
, layoutRatios("layoutRatios")
, alignment("alignment", Alignment::EVEN)
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
   if constexpr (VERBOSE) Logger::debug() << child->getName() << " added to layout " << getName() << std::endl;
   if (layoutRatios.size() < getChildren().size()){
      layoutRatios.append(1.0);
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
   if (layoutRatios.size() > getChildren().size()){
      layoutRatios.pop_back();
   }
}

void Layout::renderEnd() {
   //debug
//   drawRectangleLines({0, 0, getWidth(), getHeight()}, 1.0, COLORS::black);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::arrangeChildren() {
   if (getChildren().empty()) return;
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
      if (getWidth() && getHeight()) {
         for (int i = 0; i < getChildren().size(); i++) {
            auto &child = getChildren().at(i);
            auto subrect= getRect().toSizeRect().getSubRect(boundingBox, i);
            child->setRect(subrect);
         }
      }
   } else {
      const auto expandingDimension = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
      // For front and back alignment, set all widgets to their minimum/maximum size, then allocate the leftover space to nothing.
      switch (alignment.value){
         case Alignment::FRONT:{
            R_FLOAT pos = 0;
            for (int i=0; i<getChildren().size(); i++){
               //just make the thing as small as possible - setpendingrect will spit back its minimum size
               LayoutHelper helper(dir, i, getChildren()[i]);
               helper.setReleventPosition(pos);
               auto pendingRect = dir == LayoutDir::HORIZONTAL ? Rect<R_FLOAT>(pos,0,0,getHeight()) : Rect<R_FLOAT>(0,pos,getWidth(),0);
               auto minSizeOpt = helper.setPendingRect(pendingRect);
               //will either be constrained, or be 0, so we don't need to check other conditions
               if (minSizeOpt && minSizeOpt.value()){
                  pos += minSizeOpt.value();
               }
            }
            break;}
         case Alignment::BACK:{
            R_FLOAT pos = dir == LayoutDir::HORIZONTAL ? getWidth() : getHeight();
            for (int i=0; i<getChildren().size(); i++){
               //just make the thing as small as possible - setpendingrect will spit back its minimum size
               LayoutHelper helper(dir, i, getChildren()[i]);
               auto pendingRect = dir == LayoutDir::HORIZONTAL ? Rect<R_FLOAT>(0,0,0,getHeight()) : Rect<R_FLOAT>(0,0,getWidth(),0);
               auto minSizeOpt = helper.setPendingRect(pendingRect);
               //will either be constrained, or be 0, so we don't need to check other conditions
               if (minSizeOpt){
                  pos -= minSizeOpt.value();
               }
               helper.setReleventPosition(pos);
            }
            break;}
         case Alignment::EVEN:{
            //how much space we will allocate to each child
            auto totalSizeToAllocate = expandingDimension;
            if constexpr (VERBOSE) Logger::debug() << "Layout " << getName() << " has " << getChildren().size() << " children" << endl;

            //determine which children we are giving space to. Children which are maxed out will not be considered
            // in this calculation. Start with a vector of all children, and remove them as we need to.
            std::vector<std::unique_ptr<LayoutHelper>> childLayoutsAll;
            std::vector<std::reference_wrapper<LayoutHelper>> childLayoutsAvailable;
            for (int i=0; i<getChildren().size(); i++){
               childLayoutsAll.emplace_back(make_unique<LayoutHelper>(dir, i, getChildren().at(i)));
               childLayoutsAvailable.emplace_back(*childLayoutsAll.back());
            }
            auto removeLayoutFromConsideration = [&](LayoutHelper& layout){
               for (auto it = childLayoutsAvailable.begin(); it != childLayoutsAvailable.end(); it++){
                  if (layout == *it){
                     childLayoutsAvailable.erase(it);
                     return;
                  }
               }
            };

            bool startOver = false;
            if constexpr (VERBOSE) Logger::debug() << "Parent " << getName() << " allocating " << getSize() << " pixels to children!" << endl;
            do {
               for (auto& _layout: childLayoutsAvailable){
                  auto& layout = _layout.get();
                  startOver = false;
                  int allocatedSpace;
                  {
                     double denominator = 0;
                     double numerator = layoutRatios.value.at(layout.childIndex);
                     for (int i=0; i<childLayoutsAvailable.size(); i++) {
                        denominator += layoutRatios.value.at(childLayoutsAvailable.at(i).get().childIndex);
                     }
                     allocatedSpace = (int) (totalSizeToAllocate * numerator / denominator);
                     if constexpr (VERBOSE) Logger::debug() << "Child " << layout.child->getName() << " with scale of " << numerator << "/" << denominator << " can potentially be allocated " << allocatedSpace << " pixels" << endl;
                  }
                  std::optional<int> isConstrained;
                  if (dir == LayoutDir::HORIZONTAL) {
                     isConstrained = layout.setPendingRect({{0, 0},{allocatedSpace, getHeight()}});
                  } else {
                     isConstrained = layout.setPendingRect({{0, 0}, {getWidth(), allocatedSpace}});
                  }
                  if (isConstrained) {
                     removeLayoutFromConsideration(layout);
                     totalSizeToAllocate -= isConstrained.value();
                     startOver = true;
                  }
                  if (startOver) break;
               }
            } while (startOver && !childLayoutsAvailable.empty());

            // Now that the sizes of the children are correctly determined, we must place them at their appropriate positions
            int currentPos = 0;
            for (auto& layout : childLayoutsAll) {
               layout->setReleventPosition(currentPos);
               currentPos += layout->getReleventDimension();
            }
         break;}
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> Layout::calculateIdealBoundingBox() {
   Size<R_FLOAT> idealBoundingBox;
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
//            auto subrect= getRect().toSizeRect().getSubRect(boundingBox, i);
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