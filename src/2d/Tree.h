#pragma once
#include "Layout.h"
#include "ScrollView.h"
#include <variant>

namespace ReyEngine{
   class Tree;
   class TreeItem;
   /// what a cell sorts by (see TreeItem::setCell): numbers by value, before text (by letter, any case), before nothing
   using TreeSortKey = std::variant<std::monostate, int64_t, double, std::string>;
   //todo: this doesn't need to be an interface since tree doesn't do it anymore. move all this to tree item
   class TreeItemContainer {
   public:
      static constexpr size_t GENERATION_NULL = -1;
      TreeItemContainer() = default;
      TreeItem* push_back(std::unique_ptr<TreeItem>&& item);
      TreeItem* push_back(const std::string& item);
      TreeItem* insertItem(size_t atIndex, std::unique_ptr<TreeItem> item);
      TreeItem* front(){return _children.front().get();}
      TreeItem* back(){return _children.back().get();}
      virtual std::unique_ptr<TreeItem> takeItem(size_t index) = 0;
      void sort(std::function<bool(const std::unique_ptr<TreeItem>& a, const std::unique_ptr<TreeItem>& b)>& fxLessthan){
         std::sort(getChildren().begin(), getChildren().end(), fxLessthan);
      }
      void clear(); //remove all children
      std::vector<std::unique_ptr<TreeItem>>& getChildren(){return _children;}
   protected:
      std::vector<std::unique_ptr<TreeItem>> _children;
      Tree* _tree = nullptr;
      TreeItemContainer* _parent = nullptr;
      //the "level" of the branch on which this leaf appears.
      // For example, the root is always generation 0, its children are generation 1,
      // those children's children are generation 2, and so on.
      // Orphaned/null items have generation -1;
      size_t _generation = GENERATION_NULL;
   };

   class TreeItem: public TreeItemContainer, public MetaDataInterface {
   public:
      void setText(const std::string& text){_text = text;}
      [[nodiscard]] std::string getText() const {return _text;}
      /// The text in a column (see Tree::setColumns), and what it sorts by (nothing given: its text). Column 0 is the
      /// item's own text (see setText). Setting a cell doesn't sort the tree again: see Tree::resort.
      void setCell(size_t column, const std::string& text, TreeSortKey key = {});
      [[nodiscard]] std::string getCellText(size_t column) const;
      /// what a column sorts by: its key, or else its text
      [[nodiscard]] TreeSortKey getSortKey(size_t column) const;
      std::unique_ptr<TreeItem> takeItem(size_t index) override;
      [[nodiscard]] bool getExpanded(){return expanded;}
      void setExpanded(bool _expanded){expanded = _expanded;}
      [[nodiscard]] bool getExpandable(){return expandable;}
      void setExpandable(bool _expandable){expandable = _expandable;}
      [[nodiscard]] bool getEnabled(){return _enabled;}
      void setEnabled(bool enabled){_enabled = enabled;}
      void setIcon(std::unique_ptr<ReyTexture>&& icon){_icon = std::move(icon);}
      std::optional<ReyTexture*> getIcon(){return _icon.get();}
      std::unique_ptr<ReyTexture> takeIcon(){return std::move(_icon);}
      friend std::ostream& operator<<(std::ostream& os, const TreeItem& item) {os << item._text; return os;}
   protected:
      TreeItem(const std::string& text=""): _text(text){}
      template <typename T>
      TreeItem(const std::string& text="", std::optional<std::pair<std::string, T>> metaData=std::nullopt)
      : _text(text)
      {
         if (metaData) {
            auto& [key, value] = metaData.value();
            setMetaData(key, value);
         }
      }
      void setGeneration(size_t generation);
      std::string _text;
      bool _enabled = true; //used to limit interaction and gray it out.
      bool isRoot = false;
      bool expanded = true; //unexpanded tree items are visible, it's their children that are not;
      bool visible = true;
      bool expandable = true;
      size_t index; //only valid if in a tree
      std::unique_ptr<ReyTexture> _icon;
      /// the cells past column 0 (whose text is _text), and every column's sort key, by column
      struct Cell {
         std::string text;
         TreeSortKey key;
      };
      std::vector<Cell> _cells;

   private:
      friend class Tree;
      friend class TreeItemContainer;
   };

   class Tree : public Layout {
   public:
      REYENGINE_OBJECT(Tree)
      Tree()
      : Layout(Layout::LayoutDir::VERTICAL)
      {}
      Tree(std::unique_ptr<TreeItem>&& root)
      : Tree()
      {
         setRoot(std::move(root));
      }
      EVENT(EventTreeClicked, 654987984653){}};
      EVENT_ARGS(EventItemAdded, 654987984654, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemRemoved, 654987984655, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemClicked, 654987984656, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemDoubleClicked, 654987984657, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemHovered, 654987984658, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemSelected, 654987984659, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      EVENT_ARGS(EventItemDeselected, 654987984660, TreeItem*& item), item(item){}
         TreeItem*& item;
      };
      /// which column the rows are sorted by (none: the order they were put in), and which way
      enum class SortOrder {NONE, ASCENDING, DESCENDING};
      EVENT_ARGS(EventSortChanged, 654987984661, std::optional<size_t> column, SortOrder order), column(column), order(order){}
         std::optional<size_t> column;
         SortOrder order;
      };
      /// an item clicked, and the column the click was in (see setColumns)
      EVENT_ARGS(EventCellClicked, 654987984662, TreeItem*& item, size_t column), item(item), column(column){}
         TreeItem*& item;
         size_t column;
      };
      /// the player dragged a column's edge in the header
      EVENT_ARGS(EventColumnResized, 654987984663, size_t column, float width), column(column), width(width){}
         size_t column;
         float width;
      };
      /// A column of the rows (see setColumns). The first is the tree's own: its items' text, indented, with their
      /// arrows.
      struct Column {
         std::string title;
         float width = 100;     //pixels, unless it stretches
         float minWidth = 24;   //the narrowest it can be dragged to
         bool stretch = false;  //it takes what width the others leave (one column at most)
         enum class Align {LEFT, CENTER, RIGHT};
         Align align = Align::LEFT;
         bool sortable = true;  //a click on its header sorts by it
         bool resizable = true; //its right edge in the header can be dragged
         /// whether one item comes before another, sorting by it upwards; by default, by their sort keys (see
         /// TreeItem::getSortKey)
         std::function<bool(const TreeItem&, const TreeItem&)> compare;
      };

      template <typename... Args>
      static inline std::unique_ptr<TreeItem> createItem(const std::string& text={}, Args... args){
         return std::unique_ptr<TreeItem>(new TreeItem(text, std::forward<Args>(args)...));
      }
      [[nodiscard]] std::optional<TreeItem*> getRoot() const {if (root) return root.get(); return {};}
      void setHideRoot(bool hide){_hideRoot = hide; refresh();}
      bool getHideRoot() const {return _hideRoot;}
      TreeItem* setRoot(std::unique_ptr<TreeItem>&& item);
      TreeItem* setRoot(const std::string& rootName);
      void setAllowHighlight(bool allowHighlight){_allowHighlight = allowHighlight;}
      void setAllowSelect(bool allowSelect){_allowSelect = allowSelect;}
      [[nodiscard]] bool getAllowSelect() const {return _allowSelect;}
      [[nodiscard]] bool getAllowHighlight() const {return _allowHighlight;}
      [[nodiscard]] std::optional<TreeItem*> getSelected() const {return _selectedItem;}
      [[nodiscard]] std::optional<size_t> getSelectedIndex() const;
      void setHighlighted(TreeItem*, bool publish=true);
      void setHighlightedIndex(size_t visibleItemIndex, bool publish=true); //sets highlighted by its CURRENTLY VISIBLE index (not internal raw index)
      void clearHighlight(){_hoveredItem.reset();}
      void setSelected(TreeItem* selectedItem, bool publish=true);
      void setSelectedIndex(size_t visibleItemIndex, bool publish=true); //sets selected by its CURRENTLY VISIBLE index (not internal raw index)
      void clearSelection(){_selectedItem.reset();}
      Size<float> measureContents(); // Measures how big the contents of the tree are, not how big the tree itself is. Used for sizing.
      /// Pixels each row is tall; 0 (the default) is the font's size. Every row is the same height.
      void setRowHeight(float height){_rowHeight = height; refresh();}
      [[nodiscard]] float getRowHeight() const {return rowHeight();}
      /// Whether the tree makes itself as big as its contents whenever they change (when it isn't in a layout, which
      /// sizes it). On by default; turn it off for a tree given a size of its own, so it scrolls instead.
      void setSizeToContents(bool sizeToContents){_sizeToContents = sizeToContents;}
      /// Pixels each generation is indented by. When set, a single click on an item's arrow (the indent just before
      /// the item, see ItemState::arrow) expands or collapses it, as well as the double click. 0 (the default): only
      /// the double click does.
      void setIndentWidth(float width){_indentWidth = width;}
      [[nodiscard]] float getIndentWidth() const {return _indentWidth;}
      /// how a row is to be drawn (see renderItem)
      struct ItemState {
         bool hovered = false;
         bool selected = false;
         bool enabled = true;
         bool expandable = false; //it has children it can show
         bool expanded = false;
         int depth = 0; //generations below the top row shown (the root, or its children if it's hidden)
         Rect<float> arrow; //where its expand arrow goes, if the indent width is set (see setIndentWidth)
      };
      /// Show the rows in columns, under a header of their titles. With none (the default), each row is just its item's
      /// text. Clicking a sortable column's header sorts the rows by it, upwards, then downwards, then back to the
      /// order they were put in. Dragging the edge between two headers makes the column left of it wider or
      /// narrower (or, if that one stretches, the one right of it).
      void setColumns(std::vector<Column> columns);
      [[nodiscard]] const std::vector<Column>& getColumns() const {return _columns;}
      void setColumnWidth(size_t column, float width);
      /// where each column is across the rows: its left edge and width
      [[nodiscard]] std::vector<std::pair<float, float>> getColumnSpans() const;
      /// the column under a point across the rows, if any
      [[nodiscard]] std::optional<size_t> columnAt(float x) const;
      /// whether the column headers show, when there are columns. On by default.
      void setShowHeader(bool show){_showHeader = show; _windowDirty = true; updateScrollLayout();}
      /// pixels the header is tall; 0 (the default) is a row's height
      void setHeaderHeight(float height){_headerHeight = height; _windowDirty = true; updateScrollLayout();}
      [[nodiscard]] float getHeaderHeight() const {return headerHeight();}
      /// Sort each item's children by a column (none: the order they were put in), as a click on its header would.
      /// Only the order they're shown in changes: their order in the tree stays.
      void setSort(std::optional<size_t> column, SortOrder order, bool publish = false);
      [[nodiscard]] std::optional<size_t> getSortColumn() const {return _sortColumn;}
      [[nodiscard]] SortOrder getSortOrder() const {return _sortOrder;}
      /// Show only the items that pass a test, and those with any under them that do (so what passes keeps its place
      /// in the tree). None (the default) shows them all. Like sorting, it's done again only as the tree changes, or
      /// on resort.
      void setFilter(std::function<bool(const TreeItem&)> filter){_filter = std::move(filter); refresh();}
      /// sort and filter the rows again, after their cells have changed
      void resort(){refresh();}
      /// how a column's header is to be drawn (see renderHeader)
      struct HeaderState {
         bool hovered = false;
         bool pressed = false;
         SortOrder sort = SortOrder::NONE; //which way the rows are sorted by it, if they are
      };
      /// a text cut short to fit a width, ending in "..." if it's cut
      [[nodiscard]] static std::string fitText(const std::string& text, float width, const ReyEngineFont& font);
      void ensureItemVisible(TreeItem* item); //scroll so the item's row sits inside the viewport. No-op if the item isn't currently visible (collapsed/absent).
      struct Iterator {
         using iterator_category = std::forward_iterator_tag;
         using difference_type   = std::ptrdiff_t;
         using value_type        = TreeItem;
         using pointer           = TreeItem*;
         using reference         = TreeItem&;

         Iterator(pointer ptr) : iterPtr(ptr), root(ptr) {}

         reference operator*() const { return *iterPtr; }
         pointer operator->() { return iterPtr; }
         Iterator& operator++() {
            auto& _order = root->_tree->order;
            if (leafIndex >= _order.size()){
               iterPtr = nullptr;
            } else {
               iterPtr = _order[leafIndex++];
            }
            return *this;
         }
         Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
         friend bool operator== (const Iterator& a, const Iterator& b) { return a.iterPtr == b.iterPtr; };
         friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iterPtr != b.iterPtr; };

      private:
         pointer iterPtr;
         size_t leafIndex = 1;
         TreeItem* root;
      };
      Iterator begin() { ensureFresh(); return {root.get()}; } //iteration reads `order`, which is rebuilt lazily
      Iterator end()   { return {nullptr}; }

   protected:

      //Stores extra details that the tree can use. Only materialized for rows in the scroll
      //window - the rest of the tree exists as bare TreeItem pointers in _visibleRows.
      struct TreeItemImplDetails{
         static std::string buildExpansionText(TreeItem* item){
            char c = item->expandable && !item->_children.empty() ? (item->expanded ? '-' : '+') : ' ';
            long long generationOffset = item->_tree->_hideRoot ? -1 : 0;
            return c + std::string(generationOffset + item->_generation, c);
         }
         TreeItemImplDetails(TreeItem*& item, int visibleRowindex): item(item), visibleRowIndex(visibleRowindex){
            expansionRegionText = buildExpansionText(item);
            //content space - unaffected by scrolling
            expansionIconClickRegion = {{0, visibleRowindex * item->_tree->rowHeight()}, {item->_tree->getWidth(), item->_tree->rowHeight()}};
         }
         ReyEngine::Rect<float> expansionIconClickRegion; //where we can click to determine if an item should be "expanded" or not;
         std::string expansionRegionText;
         TreeItem* item;
         int visibleRowIndex = -1; //non-const: details live by value in the window vector
      };

      void refresh(); //marks the tree dirty; the actual rebuild is coalesced into ensureFresh()
      void _init() override;
      void render2D(RenderContext&) const override;
      /// Draw one row, in the tree's own space; the row is the whole width left of the scrollbar. Override to draw
      /// rows another way: the tree still does the scrolling, hovering, selecting and expanding. The default draws
      /// the hover or selection highlight, then "+"/"-" and the item's text.
      virtual void renderItem(RenderContext& context, const TreeItem& item, const Rect<float>& row, const ItemState& state) const;
      /// Draw one cell of a row, when there are columns (see setColumns): the default renderItem draws the row's
      /// highlight, then each of its cells, clipped to it. The default draws the cell's text, aligned as its column
      /// says, cut short with "..." if it doesn't fit; the first column's indented, with "+"/"-" before it.
      virtual void renderCell(RenderContext& context, const TreeItem& item, size_t column, const Rect<float>& cell, const ItemState& state) const;
      /// draw a column's header, clipped to it
      virtual void renderHeader(RenderContext& context, size_t column, const Rect<float>& rect, const HeaderState& state) const;
      //the tree renders its rows itself; its only widget child is the scrollbar, which lives
      //in a right-edge gutter instead of being arranged by the base layout.
      void arrangeChildren() override;
      Handled _unhandled_input(const InputEvent&) override;
      void _on_mouse_enter() override {}
      void _on_mouse_exit() override {_hoveredItem.reset(); _headerHovered.reset();}
      std::optional<TreeItemImplDetails*> getItemDetailsAt(const Pos<float>& localPos); //returned pointer is transient: invalidated by the next window rebuild
   private:
      void forget(TreeItem* item); //an item (and everything under it) leaving the tree: stop holding it as hovered, selected or last clicked
      void ensureFresh();  //rebuild order/_visibleRows if a mutation marked the tree dirty. O(1) when clean.
      void updateWindow(); //(re)annotate the rows in/near the viewport if the window moved or the tree changed
      //single source of truth for row height: rendering, hit-testing, and click regions must all agree
      [[nodiscard]] float rowHeight() const {return _rowHeight > 0 ? _rowHeight : theme ? theme->font->size : 0;}
      [[nodiscard]] float contentHeight() const {return _visibleRows.size() * rowHeight();}
      /// pixels down to the first row: the header's height, if it shows
      [[nodiscard]] float headerHeight() const {return _columns.empty() || !_showHeader ? 0 : _headerHeight > 0 ? _headerHeight : rowHeight();}
      /// pixels the rows have, below the header
      [[nodiscard]] float rowsHeight() const {return std::max(0.0f, getHeight() - headerHeight());}
      /// a's children as shown: those that pass the filter, sorted (see setSort, setFilter)
      [[nodiscard]] std::vector<TreeItem*> shownChildren(TreeItem* item, const std::function<bool(TreeItem*)>& keep) const;
      /// the header's column edge near a point, if it's one that can be dragged
      [[nodiscard]] std::optional<size_t> edgeAt(const Pos<float>& localPos) const;
      /// the mouse is over the header: what it's doing there
      Handled headerInput(const InputEvent& event, const Pos<float>& localPos);
      [[nodiscard]] float viewportWidth() const {return getWidth() - (_scroll.needsVBar() ? SCROLLBAR_WIDTH : 0.0f);} //width minus the gutter when the bar shows
      void updateScrollLayout(); //refresh scroll limits + bar rect from current content/viewport
      void ensureRowVisible(size_t visibleRowIndex);
      bool _allowSelect = false;
      bool _allowHighlight = true;
      std::unique_ptr<TreeItem> root;
      std::vector<TreeItem*> order; //a full accounting of the tree's contents, used by the iterator.
      std::vector<TreeItem*> _visibleRows; //every visible (expanded-ancestor) row, top to bottom. Cheap pointers only.
      //the expensive per-row annotations, materialized only for rows [_windowStart, _windowStart + _windowDetails.size())
      std::vector<TreeItemImplDetails> _windowDetails;
      size_t _windowStart = 0;
      bool _treeDirty = true;   //a mutation invalidated order/_visibleRows; rebuilt on next read
      bool _windowDirty = true; //_windowDetails needs a rebuild even if the window didn't move
      bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
      float _rowHeight = 0;   //see setRowHeight
      bool _sizeToContents = true;
      float _indentWidth = 0; //see setIndentWidth
      std::optional<TreeItem*> _hoveredItem;
      std::optional<TreeItem*> _lastClicked;
      std::optional<TreeItem*> _selectedItem;
      std::vector<Column> _columns; //see setColumns
      bool _showHeader = true;
      float _headerHeight = 0;
      std::optional<size_t> _sortColumn;
      SortOrder _sortOrder = SortOrder::NONE;
      std::function<bool(const TreeItem&)> _filter;
      std::optional<size_t> _headerHovered;
      std::optional<size_t> _headerPressed; //the column whose header the left button went down on, while it's held
      /// a column edge being dragged: the column getting wider or narrower, where the drag started, its width then,
      /// and whether it widens as the mouse goes left (the column right of a stretching one)
      struct Resize {
         size_t column;
         float startX;
         float startWidth;
         bool inverse;
      };
      std::optional<Resize> _resizing;
      static constexpr float EDGE_GRAB = 4; //pixels either side of a column's edge that grab it

      // vertical scrolling. _scroll owns the offset/limits and keeps _vScrollBar in sync;
      // the tree offsets its own row drawing and translates input by _scroll.offsetY().
      ScrollView _scroll;
      std::shared_ptr<Slider> _vScrollBar;
      static constexpr float SCROLLBAR_WIDTH = 14.0f;

      friend class TreeItem;
      friend class TreeItemContainer;
   };
}