#pragma once
#include "Layout.h"

namespace ReyEngine{
   class Tree;
   class TreeItem;
   //todo: this doesn't need to be an interface since tree doesn't do it anymore. move all this to tree item
   class TreeItemContainer {
   public:
      static constexpr size_t GENERATION_NULL = -1;
      TreeItemContainer() = default;
      TreeItem* push_back(std::unique_ptr<TreeItem>&& item);
      TreeItem* push_back(const std::string& item);
      TreeItem* insertItem(int atIndex, std::unique_ptr<TreeItem> item);
      TreeItem* front(){return _children.front().get();}
      TreeItem* back(){return _children.back().get();}
      /////////////////////////////////////////////////////////////////////////////////////////
      virtual std::unique_ptr<TreeItem> removeItem(size_t index) = 0;
      void sort(std::function<bool(const std::unique_ptr<TreeItem>& a, const std::unique_ptr<TreeItem>& b)>& fxLessthan){
         std::sort(getChildren().begin(), getChildren().end(), fxLessthan);
      }
      void clear(); //remove all children
      /////////////////////////////////////////////////////////////////////////////////////////
      std::vector<std::unique_ptr<TreeItem>>& getChildren(){return _children;}
   protected:
      std::vector<std::unique_ptr<TreeItem>> _children;
      Tree* _tree = nullptr;
      TreeItemContainer* _parent;
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
      std::unique_ptr<TreeItem> removeItem(size_t index) override;
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

   private:
      friend class Tree;
      friend class TreeItemContainer;
   };

   class Tree : public Layout {
   public:
      static constexpr float ROW_HEIGHT = 20;
      REYENGINE_OBJECT(Tree)
      Tree() : Layout(Layout::LayoutDir::VERTICAL){
         acceptsHover = true;
      }
      Tree(std::unique_ptr<TreeItem>&& root) : Tree()
      {
         setRoot(std::move(root));
      }
      EVENT(EventTreeClicked, 654987984653){}
      };
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

      template <typename... Args>
      static inline std::unique_ptr<TreeItem> createItem(const std::string& text={}, Args... args){
         return std::unique_ptr<TreeItem>(new TreeItem(text, std::forward<Args>(args)...));
      }
      [[nodiscard]] std::optional<TreeItem*> getRoot() const {if (root) return root.get(); return {};}
      void setHideRoot(bool hide){_hideRoot = hide; determineVisible();}
      TreeItem* setRoot(std::unique_ptr<TreeItem>&& item);
      TreeItem* setRoot(const std::string& rootName);
      std::unique_ptr<TreeItem> takeItem(std::unique_ptr<TreeItem>&&); //takes ownership of another tree item
      void setAllowHighlight(bool allowHighlight){_allowHighlight = allowHighlight;}
      void setAllowSelect(bool allowSelect){_allowSelect = allowSelect;}
      [[nodiscard]] bool getAllowSelect(){return _allowSelect;}
      [[nodiscard]] bool getAllowHighlight(){return _allowHighlight;}
      std::optional<TreeItem*> getSelected(){return _selectedItem;}
      std::optional<size_t> getSelectedIndex();
      void setHighlighted(TreeItem*, bool publish=true);
      void setHighlightedIndex(size_t visibleItemIndex, bool publish=true); //sets highlighted by its CURRENTLY VISIBLE index (not internal raw index)
      void setSelected(TreeItem* selectedItem, bool publish=true);
      void setSelectedIndex(size_t visibleItemIndex, bool publish=true); //sets selected by its CURRENTLY VISIBLE index (not internal raw index)
      Size<float> measureContents(); // Measures how big the contents of the tree are, not how big the tree itself is. Used for sizing.
      void fit(){setSize(measureContents());}
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
      Iterator begin() { return {root.get()}; }
      Iterator end()   { return {nullptr}; }

   protected:

      //Stores extra details that the tree can use
      struct TreeItemImplDetails{
         TreeItemImplDetails(TreeItem*& item, int visibleRowindex): item(item), visibleRowIndex(visibleRowindex){}
         ReyEngine::Rect<float> expansionIconClickRegion; //where we can click to determine if an item should be "expanded" or not;
         TreeItem* item;
         const int visibleRowIndex = -1;
      };

      void determineOrdering();
      void determineVisible();
      void render2D() const override;
      Widget* _unhandled_input(const InputEvent&) override;
      void _on_mouse_enter() override {};
      void _on_mouse_exit() override { _hoveredImplDetails.reset();}
      std::optional<TreeItemImplDetails*> getImplDetailsAt(const Pos<float>& localPos);
   private:
      bool _allowSelect = false;
      bool _allowHighlight = true;
      std::unique_ptr<TreeItem> root;
      std::vector<TreeItem*> order; //a full accounting of the tree's contents, used by the iterator.
      std::vector<TreeItemImplDetails*> _visibleItems; //a
      bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
      std::optional<TreeItemImplDetails*> _hoveredImplDetails;
      std::optional<TreeItem*> _lastClicked;
      std::optional<TreeItem*> _selectedItem;

      friend class TreeItem;
      friend class TreeItemContainer;
   };
}