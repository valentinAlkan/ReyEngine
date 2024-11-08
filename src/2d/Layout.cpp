#include "Layout.h"
#include "MiscTools.h"
#include "numeric"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Layout::Layout(const std::string &name, const std::string &typeName, LayoutDir layoutDir)
: BaseWidget(name, typeName)
, NamedInstance(name, typeName)
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
      if (_rect.value.size().x && _rect.value.size().y) {
         for (int i = 0; i < getChildren().size(); i++) {
            auto &child = getChildren().at(i);
            auto subrect = _rect.value.toSizeRect().getSubRect(boundingBox, i);
            child->setRect(subrect);
         }
      }
   } else {
      //how much space we have to allocate
      auto totalSpace = dir == LayoutDir::HORIZONTAL ? _rect.value.width : _rect.value.height;
      //how much space we will allocate to each child
      auto totalSizeToAllocate = totalSpace;
      Logger::debug() << "Layout " << getName() << " has " << getChildren().size() << " children" << endl;

      //new method
      //determine which children we are giving space to. Children which are maxed out will not be considered
      // in this calculation. Start with a vector of all children, and remove them as we need to.
      struct LayoutHelper {
         LayoutHelper(LayoutDir layoutDir, int index, std::shared_ptr<BaseWidget>& child)
         : childIndex(index)
         , layoutDir(layoutDir)
         , child(child)
         {}
         ~LayoutHelper(){
            //apply the rect
            Logger::info() << "Parent " << child->getParent().lock()->getName() << " applying rectangle " << pendingRect << " to child " << child->getName() << endl;
            //apply margins - this should always be the very last thing we do
            auto theme = child->getParent().lock()->getTheme();
            pendingRect.x += theme->layoutMargins.left();
            pendingRect.y += theme->layoutMargins.top();
            pendingRect.width -= (theme->layoutMargins.right() + theme->layoutMargins.left());
            pendingRect.height -= (theme->layoutMargins.bottom() + theme->layoutMargins.top());
            child->setRect(pendingRect);
         }
         /// Accounts for min/max
         std::optional<int> setPendingRect(const Rect<int>& newRect){
            Tools::AnonymousDtor dtor([&](){Logger::info() << "Child " << child->getName() << " will be allowed " << pendingRect.size() << " space" << endl;});
            pendingRect = newRect;
            auto maxSize = layoutDir == LayoutDir::HORIZONTAL ? child->getMaxSize().x : child->getMaxSize().y;
            auto minSize = layoutDir == LayoutDir::HORIZONTAL ? child->getMinSize().x : child->getMinSize().y;
            auto& releventDimension = layoutDir == LayoutDir::HORIZONTAL ? pendingRect.width : pendingRect.height;

            bool isConstrainedMax = releventDimension >= maxSize;
            bool isConstrainedMin = releventDimension <= minSize;
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
         void setReleventPosition(int pos) {
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
         Rect<int> pendingRect;
      };
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
      Logger::info() << "Parent " << getName() << " allocating " << getSize() << " pixels to children!" << endl;
      do {
         for (auto& _layout: childLayoutsAvailable){
            auto& layout = _layout.get();
            auto& child = layout.child;
            startOver = false;
//            Logger::info() << "Child " << layout.childIndex + 1 << " of " << childLayoutsAvailable.size() << " will have a position of " << currentPos << endl;
            int allocatedSpace;
            {
               double denominator = 0;
               double numerator = childScales.value.at(layout.childIndex);
               for (int i=0; i<childLayoutsAvailable.size(); i++) {
                  denominator += childScales.value.at(childLayoutsAvailable.at(i).get().childIndex);
               }
               allocatedSpace = (int) (totalSizeToAllocate * numerator / denominator);
               Logger::info() << "Child " << child->getName() << " with scale of " << numerator << "/" << denominator << " can potentially be allocated " << allocatedSpace << " pixels" << endl;
            }
            std::optional<int> isConstrained;
            if (dir == LayoutDir::HORIZONTAL) {
               isConstrained = layout.setPendingRect({{0, 0},{allocatedSpace, _rect.value.height}});
            } else {
               isConstrained = layout.setPendingRect({{0, 0}, {_rect.value.width, allocatedSpace}});
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