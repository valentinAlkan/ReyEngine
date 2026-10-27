#include "Tree.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////////
void TreeItemContainer::clear() {
   if (_tree) for (auto& child : _children) _tree->forget(child.get());
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
   if (_tree) _tree->forget(ptr.get());

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
void TreeItem::setCell(size_t column, const std::string& text, TreeSortKey key) {
   if (_cells.size() <= column) _cells.resize(column + 1);
   if (column == 0) _text = text;
   else _cells[column].text = text;
   _cells[column].key = std::move(key);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string TreeItem::getCellText(size_t column) const {
   if (column == 0) return _text;
   return column < _cells.size() ? _cells[column].text : std::string();
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeSortKey TreeItem::getSortKey(size_t column) const {
   if (column < _cells.size() && !std::holds_alternative<std::monostate>(_cells[column].key)) return _cells[column].key;
   auto text = getCellText(column);
   if (text.empty()) return {};
   return text;
}

namespace {
   /// how two sort keys compare: below 0 if a comes first, above if b does. Nothing comes last either way round.
   int compareKeys(const TreeSortKey& a, const TreeSortKey& b, bool descending) {
      bool aNone = std::holds_alternative<std::monostate>(a), bNone = std::holds_alternative<std::monostate>(b);
      if (aNone || bNone) return aNone == bNone ? 0 : aNone ? 1 : -1;
      auto number = [](const TreeSortKey& key) -> std::optional<double> {
         if (auto value = std::get_if<int64_t>(&key)) return (double)*value;
         if (auto value = std::get_if<double>(&key)) return *value;
         return std::nullopt;
      };
      auto aNumber = number(a), bNumber = number(b);
      int result;
      if (aNumber && bNumber) result = aNumber.value() < bNumber.value() ? -1 : aNumber.value() > bNumber.value() ? 1 : 0;
      else if (aNumber || bNumber) result = aNumber ? -1 : 1; //numbers before text
      else {
         auto& aText = std::get<std::string>(a);
         auto& bText = std::get<std::string>(b);
         result = 0;
         for (size_t k = 0; k < std::min(aText.size(), bText.size()) && !result; k++) {
            int ca = std::tolower((unsigned char)aText[k]), cb = std::tolower((unsigned char)bText[k]);
            result = ca < cb ? -1 : ca > cb ? 1 : 0;
         }
         if (!result) result = aText.size() < bText.size() ? -1 : aText.size() > bText.size() ? 1 : 0;
      }
      return descending ? -result : result;
   }
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
      Rect<float> barRect = {getWidth() - SCROLLBAR_WIDTH, headerHeight(), SCROLLBAR_WIDTH, rowsHeight()};
      layoutApplyRect(_vScrollBar.get(), barRect);
   }
   _scroll.layout(contentHeight(), rowsHeight()); //clamps offset + syncs bar range/page/visibility
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::ensureRowVisible(size_t visibleRowIndex) {
   updateScrollLayout(); //limits must reflect the latest content before we test the row
   float top = visibleRowIndex * rowHeight();
   _scroll.ensureVisibleY(top, top + rowHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::forget(TreeItem* item) {
   //the item, or anything under it, held as hovered, last clicked or selected would dangle once it's gone
   auto within = [item](std::optional<TreeItem*>& held) {
      if (!held) return;
      for (TreeItem* at = held.value(); at; at = static_cast<TreeItem*>(at->_parent)) { //every container in a tree is an item
         if (at == item) {held.reset(); return;}
      }
   };
   within(_hoveredItem);
   within(_lastClicked);
   within(_selectedItem);
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
   std::function<void(TreeItem*)> flatten = [&](TreeItem* item){
      order.push_back(item);
      item->index = i++;
      for (auto& child : item->_children) flatten(child.get());
   };
   if (root) flatten(root.get());
   //the rows, as shown: filtered, and each item's children sorted (see shownChildren). An item's kept if it passes
   //the filter, or anything under it does, worked out once each per rebuild.
   std::unordered_map<TreeItem*, bool> kept;
   std::function<bool(TreeItem*)> keep = [&](TreeItem* item) -> bool {
      if (!_filter || item->isRoot) return true;
      if (auto it = kept.find(item); it != kept.end()) return it->second;
      bool result = _filter(*item);
      for (auto& child : item->_children) result = keep(child.get()) || result;
      kept[item] = result;
      return result;
   };
   std::function<void(TreeItem*)> addRows = [&](TreeItem* item){
      if (!(item->isRoot && _hideRoot)) _visibleRows.push_back(item);
      if (!item->expanded) return;
      for (auto child : shownChildren(item, keep)) addRows(child);
   };
   if (root) addRows(root.get());

   //drop the selection/hover if their items are no longer visible
   auto stillVisible = [&](TreeItem* item){
      return std::find(_visibleRows.begin(), _visibleRows.end(), item) != _visibleRows.end();
   };
   if (_selectedItem && !stillVisible(_selectedItem.value())) _selectedItem.reset();
   if (_hoveredItem && !stillVisible(_hoveredItem.value())) _hoveredItem.reset();

   //make ourselves larger if we need to
   auto parent = getParentWidget();
   if (_sizeToContents && (!parent || parent && !parent.value()->isLayout())){
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
      rowCount = std::min((size_t)(rowsHeight() / rh) + 2, _visibleRows.size() - firstRow);
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
void Tree::render2D(RenderContext& context) const{
   //lazy cache fill from the const render path (the engine doesn't guarantee _process):
   //flush any pending structural rebuild + window move before reading row state
   const_cast<Tree*>(this)->updateWindow();
   const float rh = rowHeight();
   const float offsetY = _scroll.offsetY();
   const float header = headerHeight();
   auto mouse = getLocalMousePos();
   {
      //clip rows to the area under the header, left of the scrollbar gutter (inset when the bar shows)
      ScopeScissor scissor(getGlobalTransform(), Rect<float>(0, header, viewportWidth(), rowsHeight()));
      //the hovered item is only updated as the mouse moves over the tree, so it goes stale once the mouse leaves, or
      //the rows move under it (scrolled, opened, removed): it only counts while the mouse is really over its row
      for (const auto& itemMeta : _windowDetails) {
         auto& item = itemMeta.item;
         Pos<float> pos = {0, header + itemMeta.visibleRowIndex * rh - offsetY};
         if (pos.y >= getHeight()) break; //below the viewport
         Rect<float> row = {pos, {viewportWidth(), rh}};
         ItemState state;
         state.hovered = _hoveredItem && _hoveredItem.value() == item && row.contains(mouse) && mouse.y >= header && mouse.y < getHeight();
         state.selected = _selectedItem && _selectedItem.value() == item;
         state.enabled = item->_enabled;
         state.expandable = item->expandable && !item->_children.empty();
         state.expanded = item->expanded;
         state.depth = (int)item->_generation - (_hideRoot ? 1 : 0);
         state.arrow = {state.depth * _indentWidth, pos.y, _indentWidth, rh};
         renderItem(context, *item, row, state);
      }
   }
   if (header <= 0) return;
   auto spans = getColumnSpans();
   for (size_t column = 0; column < _columns.size(); column++) {
      Rect<float> rect = {spans[column].first, 0, spans[column].second, header};
      HeaderState state;
      state.hovered = _headerHovered == column && !_resizing;
      state.pressed = _headerPressed == column;
      if (_sortColumn == column) state.sort = _sortOrder;
      ScopeScissor scissor(getGlobalTransform(), rect);
      renderHeader(context, column, rect, state);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::renderItem(RenderContext& context, const TreeItem& item, const Rect<float>& row, const ItemState& state) const {
   auto& font = theme->font;
   if (_allowHighlight) {
      //highlight the selected row, or else the hovered one
      std::optional<ColorRGBA> highlightColor;
      if (state.selected) highlightColor = getTheme().background.colorHighlight;
      else if (state.hovered) highlightColor = getTheme().background.colorActive1;
      if (highlightColor) drawRectangle(row, highlightColor.value());
   }
   if (!_columns.empty()) {
      auto spans = getColumnSpans();
      for (size_t column = 0; column < _columns.size(); column++) {
         Rect<float> cell = {spans[column].first, row.y, spans[column].second, row.height};
         ScopeScissor scissor(getGlobalTransform(), cell);
         renderCell(context, item, column, cell, state);
      }
      return;
   }
   auto enabledColor = font->color;
   constexpr ReyEngine::ColorRGBA disabledColor = {127, 127, 127, 255};
   if (!state.enabled) {
      font->color = disabledColor;
   }
   drawText(TreeItemImplDetails::buildExpansionText(const_cast<TreeItem*>(&item)) + item.getText(), row.pos(), font);
   if (!state.enabled) {
      font->color = enabledColor;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::renderCell(RenderContext&, const TreeItem& item, size_t column, const Rect<float>& cell, const ItemState& state) const {
   constexpr float PADDING = 4;
   auto& font = theme->font;
   std::string text = item.getCellText(column);
   float left = cell.x + PADDING;
   if (column == 0) {
      if (_indentWidth > 0) left = cell.x + (state.depth + 1) * _indentWidth;
      else text = TreeItemImplDetails::buildExpansionText(const_cast<TreeItem*>(&item)) + text;
   }
   float width = cell.x + cell.width - PADDING - left;
   text = fitText(text, width, *font);
   float textWidth = measureText(text, font).x;
   float x = left;
   switch (_columns[column].align) {
      case Column::Align::LEFT: break;
      case Column::Align::CENTER: x = left + (width - textWidth) / 2; break;
      case Column::Align::RIGHT: x = left + width - textWidth; break;
   }
   auto enabledColor = font->color;
   if (!state.enabled) font->color = ColorRGBA(127, 127, 127, 255);
   drawText(text, {x, cell.y + (cell.height - font->size) / 2}, font);
   font->color = enabledColor;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::renderHeader(RenderContext&, size_t column, const Rect<float>& rect, const HeaderState& state) const {
   constexpr float PADDING = 4;
   constexpr float ARROW = 4; //half the sort triangle's width
   auto& font = theme->font;
   drawRectangle(rect, state.pressed || state.hovered ? getTheme().background.colorActive1 : getTheme().background.colorSecondary);
   float textWidth = rect.width - 2 * PADDING - (state.sort != SortOrder::NONE ? 2 * ARROW + PADDING : 0);
   drawText(fitText(_columns[column].title, textWidth, *font), {rect.x + PADDING, rect.y + (rect.height - font->size) / 2}, font);
   if (state.sort != SortOrder::NONE) {
      float cx = rect.x + rect.width - PADDING - ARROW, cy = rect.y + rect.height / 2;
      if (state.sort == SortOrder::ASCENDING) DrawTriangle({cx, cy - ARROW}, {cx - ARROW, cy + ARROW}, {cx + ARROW, cy + ARROW}, font->color);
      else DrawTriangle({cx - ARROW, cy - ARROW}, {cx, cy + ARROW}, {cx + ARROW, cy - ARROW}, font->color);
   }
   drawLine({{rect.x + rect.width - 1, rect.y + 2}, {rect.x + rect.width - 1, rect.y + rect.height - 2}}, 1, getTheme().background.colorPrimary);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Tree::fitText(const std::string& text, float width, const ReyEngineFont& font) {
   if (measureText(text, font).x <= width) return text;
   //the longest start of it that fits with the dots (cut between whole UTF-8 characters)
   std::string dots = "...";
   size_t lo = 0, hi = text.size();
   while (lo < hi) {
      size_t mid = (lo + hi + 1) / 2;
      if (measureText(text.substr(0, mid) + dots, font).x <= width) lo = mid;
      else hi = mid - 1;
   }
   while (lo > 0 && lo < text.size() && ((unsigned char)text[lo] & 0xC0) == 0x80) lo--;
   return lo == 0 && measureText(dots, font).x > width ? std::string() : text.substr(0, lo) + dots;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::setColumns(std::vector<Column> columns) {
   _columns = std::move(columns);
   if (_sortColumn && _sortColumn.value() >= _columns.size()) {
      _sortColumn.reset();
      _sortOrder = SortOrder::NONE;
   }
   _headerHovered.reset();
   _headerPressed.reset();
   _resizing.reset();
   refresh();
   updateScrollLayout();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::setColumnWidth(size_t column, float width) {
   if (column >= _columns.size()) return;
   _columns[column].width = std::max(width, _columns[column].minWidth);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::pair<float, float>> Tree::getColumnSpans() const {
   std::vector<std::pair<float, float>> spans;
   float fixed = 0;
   for (auto& column : _columns) if (!column.stretch) fixed += column.width;
   float x = 0;
   bool stretched = false;
   for (auto& column : _columns) {
      float width = column.width;
      if (column.stretch && !stretched) {
         width = std::max(column.minWidth, viewportWidth() - fixed);
         stretched = true;
      }
      spans.emplace_back(x, width);
      x += width;
   }
   return spans;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> Tree::columnAt(float x) const {
   auto spans = getColumnSpans();
   for (size_t column = 0; column < spans.size(); column++) {
      if (x >= spans[column].first && x < spans[column].first + spans[column].second) return column;
   }
   return std::nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::setSort(std::optional<size_t> column, SortOrder order, bool publishIt) {
   if (!column || order == SortOrder::NONE || column.value() >= _columns.size()) {
      column.reset();
      order = SortOrder::NONE;
   }
   _sortColumn = column;
   _sortOrder = order;
   refresh();
   if (publishIt) publish(EventSortChanged(this, _sortColumn, _sortOrder));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<TreeItem*> Tree::shownChildren(TreeItem* item, const std::function<bool(TreeItem*)>& keep) const {
   std::vector<TreeItem*> children;
   for (auto& child : item->_children) if (keep(child.get())) children.push_back(child.get());
   if (!_sortColumn || _sortOrder == SortOrder::NONE) return children;
   auto column = _sortColumn.value();
   bool descending = _sortOrder == SortOrder::DESCENDING;
   auto& compare = _columns[column].compare;
   //stable, so those that sort the same keep the order they were put in
   std::stable_sort(children.begin(), children.end(), [&](TreeItem* a, TreeItem* b) {
      if (compare) return descending ? compare(*b, *a) : compare(*a, *b);
      return compareKeys(a->getSortKey(column), b->getSortKey(column), descending) < 0;
   });
   return children;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> Tree::edgeAt(const Pos<float>& localPos) const {
   if (localPos.y < 0 || localPos.y >= headerHeight()) return std::nullopt;
   auto spans = getColumnSpans();
   for (size_t column = 0; column < spans.size(); column++) {
      float edge = spans[column].first + spans[column].second;
      if (std::abs(localPos.x - edge) > EDGE_GRAB) continue;
      //a stretching column's edge moves the one right of it
      if (_columns[column].stretch) {
         if (column + 1 < _columns.size() && _columns[column + 1].resizable) return column;
         continue;
      }
      if (_columns[column].resizable) return column;
   }
   return std::nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Tree::headerInput(const InputEvent& event, const Pos<float>& localPos) {
   bool inHeader = localPos.y >= 0 && localPos.y < headerHeight() && localPos.x >= 0 && localPos.x < viewportWidth();
   _headerHovered = inHeader ? columnAt(localPos.x) : std::nullopt;
   if (!event.isEvent<InputEventMouseButton>()) return inHeader ? this : nullptr;
   const auto& button = event.toEvent<InputEventMouseButton>();
   if (button.button != InputInterface::MouseButton::LEFT || !inHeader) return nullptr;
   if (button.isDown) {
      if (auto edge = edgeAt(localPos)) {
         bool inverse = _columns[edge.value()].stretch;
         size_t column = inverse ? edge.value() + 1 : edge.value();
         _resizing = Resize{column, localPos.x, getColumnSpans()[column].second, inverse};
      } else _headerPressed = columnAt(localPos.x);
      return this;
   }
   auto pressed = _headerPressed;
   _headerPressed.reset();
   auto column = columnAt(localPos.x);
   if (!pressed || pressed != column || !_columns[column.value()].sortable) return this;
   //upwards, then downwards, then as they were put in
   if (_sortColumn != column) setSort(column, SortOrder::ASCENDING, true);
   else if (_sortOrder == SortOrder::ASCENDING) setSort(column, SortOrder::DESCENDING, true);
   else setSort(std::nullopt, SortOrder::NONE, true);
   return this;
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
          auto localPos = mouseEvent->getLocalPos();
          //a column edge being dragged follows the mouse wherever it goes, until the button comes up
          if (_resizing) {
             auto& resize = _resizing.value();
             float dx = localPos.x - resize.startX;
             setColumnWidth(resize.column, resize.startWidth + (resize.inverse ? -dx : dx));
             if (event.isEvent<InputEventMouseButton>() && !event.toEvent<InputEventMouseButton>().isDown) {
                publish(EventColumnResized(this, resize.column, _columns[resize.column].width));
                _resizing.reset();
             }
             return this;
          }
          if (!mouseEvent->isInside()) {
             _headerHovered.reset();
             _headerPressed.reset();
             return nullptr;
          }
          if (headerHeight() > 0 && localPos.y < headerHeight()) {
             _hoveredItem.reset();
             return headerInput(event, localPos);
          }
          _headerHovered.reset();
          if (event.isEvent<InputEventMouseButton>() && !event.toEvent<InputEventMouseButton>().isDown) _headerPressed.reset();

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
                if (!btnEvent.isDown) _lastClicked.reset();
                if (_selectedItem) {
                   EventItemDeselected itemDeSelectedEvent(this, _selectedItem.value());
                   _selectedItem.reset();
                   publish(itemDeSelectedEvent);
                }
                return nullptr;
             }
             auto itemAtClick = implDetailsAt.value()->item;
             //expand/shrink branch
             if (!btnEvent.isDown && _indentWidth > 0 && !itemAtClick->_children.empty() && itemAtClick->getExpandable()) {
                //a single click on its arrow, if rows have one
                float arrowLeft = ((float)itemAtClick->_generation - (_hideRoot ? 1 : 0)) * _indentWidth;
                if (localPos.x >= arrowLeft && localPos.x < arrowLeft + _indentWidth) {
                   itemAtClick->setExpanded(!itemAtClick->getExpanded());
                   refresh();
                   _lastClicked.reset(); //so a quick second click on the arrow toggles it again, rather than counting as a double click
                   return this;
                }
             }
             if (!btnEvent.isDown) {
//                Logger::debug() << "click at = " << itemAtClick->_text << endl;
                //publish on item click
                EventItemClicked itemClickedEvent(this, itemAtClick);
                publish(itemClickedEvent);
                if (auto column = columnAt(localPos.x); column && !_columns.empty()) {
                   EventCellClicked cellClickedEvent(this, itemAtClick, column.value());
                   publish(cellClickedEvent);
                }
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
                      if (implDetailsAt.value()->expansionIconClickRegion.contains(localPos + Pos<float>(0, _scroll.offsetY() - headerHeight()))) {
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
             //the item this click ended on, for the next to compare with: a double click only counts on the item
             //the click before it was on too (the engine only knows the two clicks were close in time and place)
             if (!btnEvent.isDown) {
                if (auto stillThere = getItemDetailsAt(localPos)) _lastClicked = stillThere.value()->item;
                else _lastClicked.reset();
             }

             return this;
          }
       }
       return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* Tree::setRoot(std::unique_ptr<TreeItem>&& item) {
    _hoveredItem.reset(); //they were the old root's
    _lastClicked.reset();
    _selectedItem.reset();
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
   if (localPos.y < headerHeight()) return nullopt; //in the header
   auto contentY = localPos.y - headerHeight() + _scroll.offsetY(); //translate viewport space -> content space
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
      retval.y += _rowHeight > 0 ? _rowHeight : itemSize.y;
   }
   //in columns: as wide as they are, and the header on top
   if (!_columns.empty()) {
      retval.x = 0;
      for (auto& column : _columns) retval.x += column.width;
      retval.y += headerHeight();
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