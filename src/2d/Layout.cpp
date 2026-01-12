#include "Layout.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

static constexpr bool VERBOSE = false;
static constexpr bool DEBUG_DRAW = false;
/// A struct that helps us layout widgets. Applies changes on dtor.
struct Layout::LayoutHelper {
   LayoutHelper(LayoutDir layoutDir, int childIndex, size_t totalChildrenInLayout, Layout* parent, Widget* child)
   : childIndex(childIndex)
   , layoutDir(layoutDir)
   , child(child)
   , parent(parent)
   , totalChildrenInLayout(totalChildrenInLayout)
   {}
   ~LayoutHelper(){
      //apply the rect
      if constexpr (VERBOSE) Logger::debug() << "Parent " << parent->getNode()->getName() << " applying rectangle " << pendingRect << " to child " << child->getNode()->getName() << endl;

      //apply margins - this should always be the very last thing we do
      enum class ChildOrder {FIRST, LAST, MIDDLE, ONLY, GRID_MIDDLE, GRID_LEFT, GRID_RIGHT, GRID_TOP, GRID_BOTTOM};
      auto& theme = parent->getTheme();
      //figure out where this child is in the layout relative to its siblings
      ChildOrder childOrder;
      if (totalChildrenInLayout == 1) childOrder = ChildOrder::ONLY;
      else if (childIndex == 0 && totalChildrenInLayout > 1) childOrder = ChildOrder::FIRST;
      else if (childIndex != totalChildrenInLayout - 1) childOrder = ChildOrder::MIDDLE;
      else if (childIndex == totalChildrenInLayout - 1  && totalChildrenInLayout > 1) childOrder = ChildOrder::LAST;
      float subtractAmt = theme.layoutMargin / 2; //how many pixels need to be subtracted from each pending rect
      switch (childOrder){
         case ChildOrder::FIRST:
            if (layoutDir == LayoutDir::HORIZONTAL) pendingRect.width -= subtractAmt;
            else if (layoutDir == LayoutDir::VERTICAL) pendingRect.height -= subtractAmt;
            break;
         case ChildOrder::MIDDLE:
            if (layoutDir == LayoutDir::HORIZONTAL) {
               pendingRect.x += subtractAmt;
               pendingRect.width -= subtractAmt;
            }
            else if (layoutDir == LayoutDir::VERTICAL){
               pendingRect.y += subtractAmt;
               pendingRect.height -= subtractAmt;
            }
            break;
         case ChildOrder::LAST:
            if (layoutDir == LayoutDir::HORIZONTAL) {
               pendingRect.x += subtractAmt;
               pendingRect.width -= subtractAmt;
            }
            else if (layoutDir == LayoutDir::VERTICAL) {
               pendingRect.y += subtractAmt;
               pendingRect.height -= subtractAmt;
            }
            break;
         case ChildOrder::ONLY:
            break;
         default:
            throw std::runtime_error("Grid layout margins not implemented!");
      }

      parent->layoutApplyRect(child, pendingRect);
   }
   /// Accounts for min/max
   std::optional<R_FLOAT> setPendingRect(const Rect<R_FLOAT>& newRect){
      Tools::ScopeExit exit([&](){if constexpr (VERBOSE) Logger::debug() << "Child " << child->getNode()->getName() << " will be allowed " << pendingRect.size() << " space" << endl;});
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
   /// Sets the x or y value appropriately based on the layout
   void setRelevantPosition(R_FLOAT pos) {
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
   const size_t childIndex;
   const size_t totalChildrenInLayout;
   const LayoutDir layoutDir;
   Widget* child;
   Layout* parent;
private:
   Rect<R_FLOAT> pendingRect;
};

/////////////////////////////////////////////////////////////////////////////////////////
Layout::Layout(LayoutDir layoutDir)
: alignment(Alignment::EVEN)
, layoutDir(layoutDir)
{
   _usesLayoutRatios = true;
   isLayout = true;
   if (DEBUG_DRAW) DEBUG_COLOR = ColorRGBA::random(255);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_added_to_tree(TypeNode* child) {
   if (auto isWidget = child->as<Widget>()) {
      if constexpr (VERBOSE) Logger::debug() << child->getName() << " added to layout " << getName() << std::endl;
      if (_usesLayoutRatios && layoutRatios.size() < getChildren().size()) {
         layoutRatios.push_back(1.0);
      }
      isWidget.value()->isLocked = true;
      arrangeChildren();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
//void Layout::_on_child_added_immediate(std::shared_ptr <BaseWidget> &child) {
//   //ensure we have the correct number of values for the amount of children we have
//   rectifyScales();
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_child_removed_from_tree(TypeNode* child) {
   if (_usesLayoutRatios && layoutRatios.size() > getChildren().size()){
      layoutRatios.pop_back();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::render2D() const {
   //debug
   if constexpr (DEBUG_DRAW) drawRectangle(getSizeRect(), DEBUG_COLOR);
}

//sets rect and makes child widget resize calls are preserved
/////////////////////////////////////////////////////////////////////////////////////////
void Layout::layoutApplyRect(Widget* widget, Rect<float>& r){
   auto oldRect = widget->getRect();
   auto minSize = widget->getMinSize();
   auto maxSize = widget->getMaxSize();
   r.clampWidth({minSize.x, maxSize.x});
   r.clampHeight({minSize.y, maxSize.y});
   widget->applyRect(r);
   widget->__on_rect_changed(oldRect, getRect(), true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Layout::_on_rect_changed() {
   _layoutArea = getSizeRect();
   arrangeChildren();
}
/////////////////////////////////////////////////////////////////////////////////////////
void Layout::arrangeChildren() {
   if (getChildren().empty()) return;
   const auto childCount = getChildren().size();
   if (layoutDir == LayoutDir::GRID){
      //divide the space into boxes, each box being large enough to exactly contain the largest child (in either dimension)
      // Center each child inside it's respective box.

      //determine box size
      Size<int> boundingBox;
      for (const auto& child : getChildren()){
         auto isWidget = child->as<Widget>();
         if (!isWidget) continue;
         auto widget = isWidget.value();
         boundingBox = boundingBox.max(widget->getMaxSize());
      }
      if (!boundingBox.x || !boundingBox.y) return; //invalid rect
      //create subrects to lay out the children
      if (getWidth()!=0 && getHeight()!=0) {
         for (int i = 0; i < childCount; i++) {
            auto child = getChildren().at(i);
            auto isWidget = child->as<Widget>();
            if (!isWidget) continue;
            auto widget = isWidget.value();
            auto subrect= getRect().toSizeRect().getSubRect(boundingBox, i);
            layoutApplyRect(widget, subrect);
         }
      }
   } else {
      const auto expandingDimension = layoutDir == LayoutDir::HORIZONTAL ? _layoutArea.width : _layoutArea.height;
      if (expandingDimension <= 0) return; //do not try to allocate for 0 sizes
      // For front and back alignment, set all widgets to their minimum/maximum size, then allocate the leftover space to nothing.
      switch (alignment){
         case Alignment::FRONT:{
            R_FLOAT pos = _layoutArea.x;
            for (int i=0; i<childCount; i++){
               //just make the thing as small as possible - setpendingrect will spit back its minimum size
               auto child = getChildren().at(i);
               auto isWidget = child->as<Widget>();
               if (!isWidget) continue;
               LayoutHelper helper(layoutDir, i, childCount, this, isWidget.value());
               helper.setRelevantPosition(pos);
               auto pendingRect = layoutDir == LayoutDir::HORIZONTAL ? Rect<R_FLOAT>(pos, _layoutArea.y, 0, _layoutArea.height) : Rect<R_FLOAT>(_layoutArea.x, pos, _layoutArea.width, 0);
               auto minSizeOpt = helper.setPendingRect(pendingRect);
               //will either be constrained, or be 0, so we don't need to check other conditions
               if (minSizeOpt && minSizeOpt.value()){
                  pos += minSizeOpt.value();
               }
            }
            break;}
         case Alignment::BACK:{
            R_FLOAT pos = layoutDir == LayoutDir::HORIZONTAL ? _layoutArea.topRight().x : _layoutArea.bottomLeft().y;
            for (int i=0; i<childCount; i++){
               //just make the thing as small as possible - setpendingrect will spit back its minimum size
               auto child = getChildren().at(i);
               auto isWidget = child->as<Widget>();
               if (!isWidget) continue;
               LayoutHelper helper(layoutDir, i, childCount, this, isWidget.value());
               auto pendingRect = layoutDir == LayoutDir::HORIZONTAL ? Rect<R_FLOAT>(_layoutArea.pos(), {0, _layoutArea.height}) : Rect<R_FLOAT>(_layoutArea.pos(), {_layoutArea.width, 0});
               auto minSizeOpt = helper.setPendingRect(pendingRect);
               //will either be constrained, or be 0, so we don't need to check other conditions
               if (minSizeOpt){
                  pos -= minSizeOpt.value();
               }
               helper.setRelevantPosition(pos);
            }
            break;}
         case Alignment::EVEN:{
            //how much space we will allocate to each child
            auto totalSizeToAllocate = expandingDimension;
            if constexpr (VERBOSE) Logger::debug() << "Layout " << getName() << " has " << childCount << " children" << endl;

            //determine which children we are giving space to. Children which are maxed out will not be considered
            // in this calculation. Start with a vector of all children, and remove them as we need to.
            // use unique ptrs in case things get reallocated
            std::vector<std::unique_ptr<LayoutHelper>> childLayoutsAll;
            std::vector<LayoutHelper*> childLayoutsAvailable;
            for (int i=0; i<childCount; i++){
               auto child = getChildren().at(i);
               auto isWidget = child->as<Widget>();
               if (!isWidget) continue;
               childLayoutsAll.emplace_back(make_unique<LayoutHelper>(layoutDir, i, childCount, this, isWidget.value()));
               childLayoutsAvailable.push_back(childLayoutsAll.back().get());
            }
            auto removeLayoutFromConsideration = [&](LayoutHelper* layout){
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
                  auto& layout = _layout;
                  startOver = false;
                  float allocatedSpace;
                  {
                     double denominator = 0;
                     double numerator = layoutRatios.at(layout->childIndex);
                     for (int i=0; i<childLayoutsAvailable.size(); i++) {
                        denominator += layoutRatios.at(childLayoutsAvailable.at(i)->childIndex);
                     }
                     allocatedSpace = totalSizeToAllocate * numerator / denominator;
                     if constexpr (VERBOSE) Logger::debug() << "Child " << layout->child->getName() << " with scale of " << numerator << "/" << denominator << " can potentially be allocated " << allocatedSpace << " pixels" << endl;
                  }
                  std::optional<int> isConstrained;
                  if (layoutDir == LayoutDir::HORIZONTAL) {
                     isConstrained = layout->setPendingRect({_layoutArea.pos(),{allocatedSpace, _layoutArea.height}});
                  } else {
                     isConstrained = layout->setPendingRect({_layoutArea.pos(), {_layoutArea.width, allocatedSpace}});
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
            int currentPos = layoutDir == LayoutDir::HORIZONTAL ? _layoutArea.x : _layoutArea.y;
            for (auto& layout : childLayoutsAll) {
               layout->setRelevantPosition(currentPos);
               currentPos += layout->getReleventDimension();
            }
         break;}
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<int> Layout::calculateIdealBoundingBox() {
   Size<R_FLOAT> idealBoundingBox;
   if (layoutDir == LayoutDir::GRID){
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
//         for (int i = 0; i < childCount; i++) {
//            auto &child = getChildren().at(i);
//            auto subrect= getRect().toSizeRect().getSubRect(boundingBox, i);
//            child->setRect(subrect);
//         }
//      }
   } else {
      if (layoutDir == LayoutDir::VERTICAL){
         for (auto& child : getChildren()){
            auto isWidget = child->as<Widget>();
            if (!isWidget) continue;

            auto childMinSize = isWidget.value()->getMinSize().max({0, 0});
            idealBoundingBox.x = Math::max(idealBoundingBox.x, childMinSize.x);
            idealBoundingBox.y += childMinSize.y;
         }
      } else {
         for (auto& child : getChildren()){
            auto isWidget = child->as<Widget>();
            if (!isWidget) continue;

            auto childMinSize = isWidget.value()->getMinSize().min({0,0});
            idealBoundingBox.x += childMinSize.x;
            idealBoundingBox.y = Math::max(idealBoundingBox.y, childMinSize.y);
         }
      }
   }
   return idealBoundingBox;
}