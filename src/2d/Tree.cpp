#include "Tree.h"
#include <algorithm>

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////////
void TreeItemContainer::clear() {
   _children.clear();
   if (_tree) _tree->refresh();
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* TreeItemContainer::push_back(std::unique_ptr<TreeItem>&& newChildItem) {
   _children.push_back(std::move(newChildItem));
   auto item = _children.back().get();
   item->_parent = this;
   item->_tree = _tree;
   item->setGeneration(_generation+1);
   if (_tree) _tree->refresh();
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
   auto retval = _children.at(atIndex).get();
   retval->_parent = this;
   retval->_tree = _tree;
   retval->setGeneration(_generation+1);
   if (_tree) _tree->refresh();
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<TreeItem> TreeItem::takeItem(size_t index){
   auto it = _children.begin() + index;
   auto ptr = std::move(*it);
   _children.erase(it);

   //item no longer is in the tree so has no parent
   ptr->_parent = nullptr;
   //remove reference to tree
   ptr->_tree = nullptr;
   ptr->_generation = TreeItem::GENERATION_NULL;
   //let the tree know to recalculate
   if (_tree) _tree->refresh();
   return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::_init() {
   //vertical scrollbar: a child Slider living in a right-edge gutter (positioned in
   //updateScrollLayout). Dragging it sets the scroll offset.
   _vScrollBar = make_child<Slider>(getNode(), std::string("__tree_vbar"), Slider::SliderType::VERTICAL);
   _vScrollBar->setVisible(false); //hidden until updateScrollLayout decides content overflows
   subscribe<Slider::EventSliderValueChanged>(_vScrollBar, [this](const auto& e){
      _scroll.setOffsetY((float)e.value, /*fromBar*/true); //user drag -> offset, don't write back to the bar
   });
   _scroll.attachVBar(_vScrollBar);
   updateScrollLayout();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::arrangeChildren() {
   //the base layout would stretch the scrollbar over the whole tree; the gutter is the
   //only child placement the tree wants, so arranging IS updating the scroll layout.
   //a resize also changes how many rows fit and how wide each row's click region is
   _windowDirty = true;
   updateScrollLayout();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::updateScrollLayout() {
   if (_vScrollBar) {
      //the bar is layout-locked (isLocked), so place it via layoutApplyRect, not setRect
      Rect<float> barRect = {getWidth() - SCROLLBAR_WIDTH, 0, SCROLLBAR_WIDTH, getHeight()};
      layoutApplyRect(_vScrollBar.get(), barRect);
   }
   _scroll.layout(contentHeight(), getHeight()); //clamps offset + syncs bar range/page/visibility
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::ensureRowVisible(size_t visibleRowIndex) {
   updateScrollLayout(); //limits must reflect the latest content before we test the row
   float top = visibleRowIndex * rowHeight();
   _scroll.ensureVisibleY(top, top + rowHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::ensureItemVisible(TreeItem* item) {
   ensureFresh();
   for (size_t row = 0; row < _visibleRows.size(); row++){
      if (_visibleRows.at(row) == item){
         ensureRowVisible(row);
         return;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::refresh(){
   //mutations are O(1): just mark dirty. The rebuild is coalesced into ensureFresh, so
   //n back-to-back push_backs cost one rebuild instead of n.
   _treeDirty = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Tree::ensureFresh() {
   if (!_treeDirty) return;
   _treeDirty = false; //clear first: measureContents below re-enters harmlessly
   _windowDirty = true;
   order.clear();
   _visibleRows.clear();
   int i=0;
   //single pre-order walk: `order` gets every item, _visibleRows gets only rows reachable
   //through expanded ancestors (minus a hidden root). Pointers only - the expensive
   //per-row annotation happens in updateWindow, and only for rows near the viewport.
   std::function<void(TreeItem*, bool)> flatten = [&](TreeItem* item, bool reachable){
      order.push_back(item);
      item->index = i++;
      if (reachable && !(item->isRoot && _hideRoot)) _visibleRows.push_back(item);
      for (auto& child : item->_children){
         flatten(child.get(), reachable && item->expanded);
      }
   };
   if (root) flatten(root.get(), true);

   //drop the selection/hover if their items are no longer visible
   auto stillVisible = [&](TreeItem* item){
      return std::find(_visibleRows.begin(), _visibleRows.end(), item) != _visibleRows.end();
   };
   if (_selectedItem && !stillVisible(_selectedItem.value())) _selectedItem.reset();
   if (_hoveredItem && !stillVisible(_hoveredItem.value())) _hoveredItem.reset();

   //make ourselves larger if we need to
   auto parent = getParentWidget();
   if (!parent || parent && !parent.value()->isLayout()){
      setSize(measureContents());
   }
   updateScrollLayout(); //row count changed -> re-clamp scroll limits + refresh the bar
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::updateWindow() {
   ensureFresh();
   //desired window: the rows intersecting the viewport, from the scroll offset
   const float rh = rowHeight();
   size_t firstRow = 0;
   size_t rowCount = _visibleRows.size();
   if (rh > 0){
      firstRow = std::min((size_t)(_scroll.offsetY() / rh), _visibleRows.size());
      rowCount = std::min((size_t)(getHeight() / rh) + 2, _visibleRows.size() - firstRow);
   }
   if (!_windowDirty && firstRow == _windowStart && rowCount == _windowDetails.size()) return;
   _windowDirty = false;
   _windowStart = firstRow;
   _windowDetails.clear();
   _windowDetails.reserve(rowCount);
   for (size_t row = firstRow; row < firstRow + rowCount; row++){
      _windowDetails.emplace_back(_visibleRows.at(row), (int)row);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::render2D(RenderContext&) const{
   //lazy cache fill from the const render path (the engine doesn't guarantee _process):
   //flush any pending structural rebuild + window move before reading row state
   const_cast<Tree*>(this)->updateWindow();
   // draw the items
   auto& font = theme->font;
   const float rh = rowHeight();
   const float offsetY = _scroll.offsetY();
   //clip rows to the area left of the scrollbar gutter (inset when the bar shows)
   ScopeScissor scissor(getGlobalTransform(), Rect<float>(0, 0, viewportWidth(), getHeight()));
   for (const auto& itemMeta : _windowDetails) {
      auto& item = itemMeta.item;
      Pos<float> pos = {0, itemMeta.visibleRowIndex * rh - offsetY};
      if (pos.y >= getHeight()) break; //below the viewport

      if (_allowHighlight) {
         //highlight the hovered row
         std::optional<ColorRGBA> highlightColor;
         if (_selectedItem && _selectedItem.value() == item) {
            //highlight the selected row
            highlightColor = Colors::blue;
         } else if (_hoveredItem && _hoveredItem.value() == item) {
            highlightColor = Colors::gray;
         }
         if (highlightColor) drawRectangle({pos, {viewportWidth(), rh}}, highlightColor.value());
      }

      auto enabledColor = font->color;
      constexpr ReyEngine::ColorRGBA disabledColor = {127, 127, 127, 255};
      if (!item->_enabled) {
         font->color = disabledColor;
      }
      drawText(itemMeta.expansionRegionText + item->getText(), pos, font);
      if (!item->_enabled) {
         font->color = enabledColor;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Tree::_unhandled_input(const InputEvent& event) {
   ensureFresh(); //input reads row state; flush any pending structural rebuild first
   switch (event.eventId){
      case InputEventKey::ID:{
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
                value < _visibleRows.size() - 1) //incr down
            {
               //do incr
               setSelectedIndex(value + increment);
               ensureRowVisible(value + increment); //keep the selection in view when navigating
               return this;
            }
         }
         break;}
      case InputEventMouseWheel::ID:{
         auto isMouse = event.isMouse();
         if (isMouse && isMouse.value()->isInside() && _scroll.needsVBar()) {
            constexpr float WHEEL_SPEED = 30.0f; //pixels per wheel notch
            const auto& wheelEvent = event.toEvent<InputEventMouseWheel>();
            _scroll.scrollByY(-wheelEvent.wheelMove.y * WHEEL_SPEED);
            return this;
         }
         break;}
       case InputEventMouseMotion::ID:
       case InputEventMouseButton::ID:
          auto mouseEvent = event.isMouse().value();
          if (!mouseEvent->isInside()) return nullptr;
          auto localPos = mouseEvent->getLocalPos();

          //figure out which row the cursor is in
          auto implDetailsAt = getItemDetailsAt(localPos);
          if (implDetailsAt){
             //hover is keyed by item, not by details pointer: the details are transient
             //(rebuilt whenever the window scrolls) but the item identity is stable.
             auto& hoveredItem = implDetailsAt.value()->item;
             bool newHover = _hoveredItem != std::optional<TreeItem*>(hoveredItem);
             _hoveredItem = hoveredItem;
             //item hover event
             if (newHover) {
                EventItemHovered itemHoverEvent(this, hoveredItem);
                publish(itemHoverEvent);
             }
          } else {
             _hoveredItem.reset();
          }

          //mouse click
          if (event.isEvent<InputEventMouseButton>()){
             const auto& btnEvent = event.toEvent<InputEventMouseButton>();
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
//                   if (_selectedItem != itemAtClick){ //allow re-selecting
                      EventItemSelected itemSelectedEvent(this, itemAtClick);
                      publish(itemSelectedEvent);
//                   }
                   _selectedItem = itemAtClick;
                }

                if (btnEvent.isDoubleClick && _lastClicked == itemAtClick){
                   if (!itemAtClick->_children.empty() && itemAtClick->getExpandable()){
                      //click regions are stored in content space; translate the local pos by the scroll offset
                      if (implDetailsAt.value()->expansionIconClickRegion.contains(localPos + Pos<float>(0, _scroll.offsetY()))) {
                         itemAtClick->setExpanded(!itemAtClick->getExpanded());
                         //invalidates implDetailsAt; everything below uses itemAtClick (a copy) instead
                         refresh();
                      }
                   }

//                   Logger::debug() << "double click at = " << itemAtClick->_text << endl;
                   EventItemDoubleClicked itemDoubleClickedEvent(this, itemAtClick);
                   publish(itemDoubleClickedEvent);
                }

             }
             if (auto stillThere = getItemDetailsAt(localPos)) {
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
    root.reset();
    root = std::move(item);
    root->isRoot = true;
    root->_tree = this;
    root->setGeneration(0);
   refresh();
    return root.get();
}
/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* Tree::setRoot(const std::string& rootName) {
   return setRoot(unique_ptr<TreeItem>(new TreeItem(rootName)));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::TreeItemImplDetails*> Tree::getItemDetailsAt(const Pos<float>& localPos) {
   updateWindow(); //details are only materialized for the current window; make sure it's current
   if (localPos.x >= viewportWidth()) return nullopt; //in the scrollbar gutter
   auto contentY = localPos.y + _scroll.offsetY(); //translate viewport space -> content space
   if (contentY < 0) return nullopt;
   size_t rowAt = (size_t)(contentY / rowHeight());
   if (rowAt >= _visibleRows.size()) return nullopt;
   //the window spans the viewport, so an in-bounds hit is normally inside it
   if (rowAt < _windowStart || rowAt - _windowStart >= _windowDetails.size()) return nullopt;
   return &_windowDetails.at(rowAt - _windowStart);
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
   ensureFresh();
   auto oldHovered = _hoveredItem;
   if (!highlighted){
      _hoveredItem.reset();
   } else {
      for (auto& visible: _visibleRows) {
         if (visible == highlighted) {
            _hoveredItem = visible;
            break;
         }
      }
   }

   if (_publish){
      if (_hoveredItem) {
         publish(EventItemHovered(this, _hoveredItem.value()));
      } else if (oldHovered){
         publish(EventItemDeselected(this, oldHovered.value()));
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setHighlightedIndex(size_t visibleItemIndex, bool _publish) {
   ensureFresh();
   if (visibleItemIndex >= _visibleRows.size()){
      Logger::error() << getNode()->getScenePath() << " : Unable to set highlighted item at index " << visibleItemIndex << endl;
   } else {
      setHighlighted(_visibleRows.at(visibleItemIndex), _publish);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::setSelected(ReyEngine::TreeItem* selectedItem, bool _publish) {
   ensureFresh();
   bool valid = false;
   if (selectedItem) {
      for (const auto& visibleItem: _visibleRows) {
         if (selectedItem == visibleItem) {
            valid = true;
            break;
         }
      }
   }
   if (!valid && selectedItem) {
      Logger::info() << "Unable to set invalid tree item!" << endl;
      return;
   }
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
   ensureFresh();
   if (visibleItemIndex >= _visibleRows.size()){
      Logger::error() << getNode()->getScenePath() << " : Unable to set selected item at index " << visibleItemIndex << endl;
   } else {
      setSelected(_visibleRows.at(visibleItemIndex), publish);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
Size<float> Tree::measureContents() {
   //note: called from ensureFresh (which clears the dirty flag first, so this re-entry is a no-op)
   ensureFresh();
   Size<float> retval;
   for (const auto& item : _visibleRows){
      auto itemSize = measureText(TreeItemImplDetails::buildExpansionText(item) + item->_text, theme->font);
      if (itemSize.x > retval.x){
         retval.x = itemSize.x;
      }
      retval.y += itemSize.y;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> Tree::getSelectedIndex() const {
   const_cast<Tree*>(this)->ensureFresh(); //lazy rebuild from a const read path
   if (auto item = getSelected()){
      for (size_t i = 0; i < _visibleRows.size(); i++){
         if (_visibleRows.at(i) == item.value()){
            return i;
         }
      }
   }
   return {};
}