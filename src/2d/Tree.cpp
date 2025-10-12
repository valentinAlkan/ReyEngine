#include "Tree.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////////
void TreeItemContainer::clear() {
   _children.clear();
   if (_tree) _tree->determineOrdering();
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* TreeItemContainer::push_back(std::unique_ptr<TreeItem>&& newChildItem) {
   _children.push_back(std::move(newChildItem));
   auto item = _children.back().get();
   item->_parent = this;
   item->_tree = _tree;
   item->setGeneration(_generation+1);
   if (_tree) _tree->determineOrdering();
   return item;
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem *TreeItemContainer::push_back(const std::string& name) {
   auto item = Tree::createItem(name);
   return push_back(std::move(item));
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem *TreeItemContainer::insertItem(size_t atIndex, std::unique_ptr<TreeItem> item) {
   _children.insert(_children.begin()+atIndex, std::move(item));
   return _children.at(atIndex).get();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<TreeItem> TreeItem::takeItem(size_t index){
   auto it = _children.begin() + index;
   auto ptr = std::move(*it);
   _children.erase(it);

   //item no longer is in the tree so has no parent
   ptr->_parent = nullptr;

   //let the tree know to recalculate
   if (_tree) _tree->determineOrdering();
   //remove reference to tree
   ptr->_tree = nullptr;
   ptr->_generation = TreeItem::GENERATION_NULL;
   return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineOrdering(){
   order.clear();
   int i=0;
   std::function<void(TreeItem*)> pushToVector = [&](TreeItem* item){
      order.push_back(item);
      item->index = i++;
      for (auto& child : item->_children){
         pushToVector(child.get());
      }
   };
   pushToVector(root.get());
   determineVisible();

}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineVisible() {
   //count how many rows are visible/expanded
   _visibleItems.clear();
   std::function<void(TreeItem*)> pushVisible = [&](TreeItem* item){
      //root item must not be hidden, otherwise if this item isn't root, then it's parent must be expanded
      if ((item->isRoot && !_hideRoot) || (!item->isRoot)){
         _visibleItems.push_back(new TreeItemImplDetails(item, _visibleItems.size()));
      }
      //set the rect that contains this item
      Pos<float> startPos = {0, (float)(_visibleItems.size() - 1) * ROW_HEIGHT};
      if (!_visibleItems.empty()) _visibleItems.back()->expansionIconClickRegion = {startPos, {getWidth(), ROW_HEIGHT}};
      if (item->expanded) {
         for (auto &child: item->_children) {
            pushVisible(child.get());
         }
      }
   };
   if (root) pushVisible(root.get());

   //make ourselves larger if we need to
   fit();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::render2D() const{
   // draw the items
   auto& font = theme->font;
   auto pos = Pos<float>(0,-ROW_HEIGHT);
   long long generationOffset = _hideRoot ? -1 : 0;
   size_t currentRow = 0;
   for (auto it = _visibleItems.begin(); it != _visibleItems.end(); it++) {
      auto& itemMeta = *it;
      auto& item = itemMeta->item;
      pos += Pos<float>(0, theme->font->size);

      if (_allowHighlight) {
         //highlight the hovered row
         std::optional<ColorRGBA> highlightColor;
         if (_selectedItem && _selectedItem.value() == item) {
            //highlight the selected row
            highlightColor = Colors::blue;
         } else if (_hoveredImplDetails && _hoveredImplDetails.value()->visibleRowIndex == currentRow) {
            highlightColor = Colors::gray;
         }
         if (highlightColor) drawRectangle({pos, {getWidth(), theme->font->size}}, highlightColor.value());
      }

      char c = item->expandable && !item->_children.empty() ? (item->expanded ? '-' : '+') : ' ';
      std::string expansionRegionText = c + std::string(generationOffset + item->_generation, c);
      auto enabledColor = font->color;
      constexpr ReyEngine::ColorRGBA disabledColor = {127, 127, 127, 255};
      if (!item->_enabled) {
         font->color = disabledColor;
      }
      drawText(expansionRegionText + item->getText(), pos, font);
      if (!item->_enabled) {
         font->color = enabledColor;
      }
      currentRow++;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Tree::_unhandled_input(const InputEvent& event) {
   switch (event.eventId){
      case InputEventKey::getUniqueEventId():{
         //up and down selection navigation
         auto& kbEvent = event.toEvent<InputEventKey>();
         if (!kbEvent.isDown || !_selectedItem) break;
         int increment;
         if (kbEvent.key == InputInterface::KeyCode::KEY_DOWN){
            increment = 1;
         } else if (kbEvent.key == InputInterface::KeyCode::KEY_UP){
            increment = -1;
         } else {
            break;
         }
         if (auto optIndex = getSelectedIndex()) {
            auto& value = optIndex.value();
            //bounds check
            if (increment < 0 && value > 0 || //incr up
                increment > 0 && value < std::numeric_limits<size_t>::max() &&
                value < _visibleItems.size() - 1) //incr down
            {
               //do incr
               setSelectedIndex(value + increment);
               return this;
            }
         }
         break;}
       case InputEventMouseMotion::getUniqueEventId():
       case InputEventMouseButton::getUniqueEventId():
          auto mouseEvent = event.isMouse().value();
          if (!mouseEvent->isInside()) return nullptr;
          auto localPos = mouseEvent->getLocalPos();

          //figure out which row the cursor is in
          auto implDetailsAt = getImplDetailsAt(localPos);
          if (implDetailsAt){
             bool newImplDetails = _hoveredImplDetails != implDetailsAt;
             _hoveredImplDetails = implDetailsAt;
             //item hover event
             if (newImplDetails) {
                EventItemHovered itemHoverEvent(this, implDetailsAt.value()->item);
                publish(itemHoverEvent);
             }
          } else {
             _hoveredImplDetails.reset();
          }

          //mouse click
          if (event.isEvent<InputEventMouseButton>()){
             auto btnEvent = event.toEvent<InputEventMouseButton>();
             if (btnEvent.button != InputInterface::MouseButton::LEFT) return nullptr;
             if (!implDetailsAt){
                if (_selectedItem) {
                   EventItemDeselected itemDeSelectedEvent(this, _selectedItem.value());
                   _selectedItem.reset();
                   publish(itemDeSelectedEvent);
                }
                return nullptr;
             }
             auto itemAtClick = implDetailsAt.value()->item;
             //expand/shrink branch
             if (!btnEvent.isDown) {
//                Logger::debug() << "click at = " << itemAtClick->_text << endl;
                //publish on item click
                EventItemClicked itemClickedEvent(this, itemAtClick);
                publish(itemClickedEvent);
                if (_allowSelect){
                   if (_selectedItem != itemAtClick){
                      EventItemSelected itemSelectedEvent(this, itemAtClick);
                      publish(itemSelectedEvent);
                   }
                   _selectedItem = itemAtClick;
                }

                if (btnEvent.isDoubleClick && _lastClicked == itemAtClick){
                   if (!itemAtClick->_children.empty() && itemAtClick->getExpandable()){
                      if (implDetailsAt.value()->expansionIconClickRegion.contains(localPos)) {
                         itemAtClick->setExpanded(!itemAtClick->getExpanded());
                         determineVisible();
                      }
                   }

//                   Logger::debug() << "double click at = " << itemAtClick->_text << endl;
                   EventItemDoubleClicked itemDoubleClickedEvent(this, itemAtClick);
                   publish(itemDoubleClickedEvent);
                }

             }
             if (auto stillThere = getImplDetailsAt(localPos)) {
                _lastClicked = stillThere.value()->item;
             } else {

             }

             return this;
          }
       }
       return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* Tree::setRoot(std::unique_ptr<TreeItem>&& item) {
    root = std::move(item);
    root->isRoot = true;
    root->_tree = this;
    root->setGeneration(0);
    determineOrdering();
    return root.get();
}
/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* Tree::setRoot(const std::string& rootName) {
   return setRoot(unique_ptr<TreeItem>(new TreeItem(rootName)));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::TreeItemImplDetails*> Tree::getImplDetailsAt(const Pos<float>& localPos) {
   auto rowHeight = theme->font->size;
   int rowAt = (int)(localPos.y / rowHeight);
   if (rowAt < _visibleItems.size()) {
      return _visibleItems.at(rowAt);
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::setGeneration(size_t generation){
   _generation = generation;
   for (auto& child : _children){
      child->_tree = _tree;
      child->setGeneration(generation + 1);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setHighlighted(ReyEngine::TreeItem* highlighted, bool _publish) {
   decltype(_hoveredImplDetails) oldHovered;
   for (auto& visible : _visibleItems){
      if (visible->item == highlighted){
         oldHovered = _hoveredImplDetails;
         _hoveredImplDetails = visible;
      }
   }

   if (_publish){
      if (_hoveredImplDetails) {
         publish(EventItemHovered(this, _hoveredImplDetails.value()->item));
      } else if (oldHovered){
         publish(EventItemDeselected(this, oldHovered.value()->item));
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setHighlightedIndex(size_t visibleItemIndex, bool _publish) {
   if (visibleItemIndex >= _visibleItems.size()){
      Logger::error() << getNode()->getScenePath() << " : Unable to set highlighted item at index " << visibleItemIndex << endl;
   } else {
      setHighlighted(_visibleItems.at(visibleItemIndex)->item, _publish);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setSelected(ReyEngine::TreeItem* selectedItem, bool _publish) {
   auto oldSelected = _selectedItem;
   _selectedItem = selectedItem;
   if (_selectedItem == nullptr) _selectedItem.reset();
   if (_publish){
      if (_selectedItem) {
         publish(EventItemSelected(this, _selectedItem.value()));
      } else if (oldSelected){
         publish(EventItemDeselected(this, oldSelected.value()));
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setSelectedIndex(size_t visibleItemIndex, bool publish){
   if (visibleItemIndex >= _visibleItems.size()){
      Logger::error() << getNode()->getScenePath() << " : Unable to set selected item at index " << visibleItemIndex << endl;
   } else {
      setSelected(_visibleItems.at(visibleItemIndex)->item, publish);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
Size<float> Tree::measureContents() {
   Size<float> retval;
   for (const auto& item : _visibleItems){
      auto itemSize = measureText(item->item->_text, theme->font);
      if (itemSize.x > retval.x){
         retval.x = itemSize.x;
      }
      retval.y += itemSize.y;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> Tree::getSelectedIndex() {
   if (auto item = getSelected()){
      int i=0;
      for (const auto& visibleItem : _visibleItems){
         if (visibleItem->item == item.value()){
            return i;
         }
         i++;
      }
   }
   return {};
}