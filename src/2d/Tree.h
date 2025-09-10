#pragma once
#include "Layout.h"

namespace ReyEngine{
   class Tree;
   class TreeItem;
   //todo: this doesn't need to be an interface since tree doesn't do it anymore. move all this to tree item
   class TreeItemContainerInterface {
   public:
      virtual TreeItem* push_back(std::unique_ptr<TreeItem>&& item) = 0;
      /////////////////////////////////////////////////////////////////////////////////////////
   //   void insertItem(int index, TreeItem*& item){getChildren().insert(getChildren().begin()+index, item);}
      /////////////////////////////////////////////////////////////////////////////////////////
      virtual std::unique_ptr<TreeItem> removeItem(size_t index) = 0;
      void sort(std::function<bool(const std::unique_ptr<TreeItem>& a, const std::unique_ptr<TreeItem>& b)>& fxLessthan){
         std::sort(getChildren().begin(), getChildren().end(), fxLessthan);
      }
      /////////////////////////////////////////////////////////////////////////////////////////
   protected:
      virtual std::vector<std::unique_ptr<TreeItem>>& getChildren() = 0;
   };

   //A struct to hold extra data for us
   struct TreeItemMeta {};
   class TreeItem: public TreeItemContainerInterface {
   public:
      static constexpr size_t GENERATION_NULL = -1;
      TreeItem(const std::string& text=""): _text(text){}
      void setText(const std::string& text){_text = text;}
      [[nodiscard]] std::string getText() const {return _text;}
      TreeItem* push_back(std::unique_ptr<TreeItem>&& newChildItem) override;
      std::unique_ptr<TreeItem> removeItem(size_t index) override;
      std::vector<std::unique_ptr<TreeItem>>& getChildren() override {return children;}
      [[nodiscard]] bool getExpanded(){return expanded;}
      void setExpanded(bool _expanded){expanded = _expanded;}
      [[nodiscard]] bool getExpandable(){return expandable;}
      void setExpandable(bool _expandable){expandable = _expandable;}
      [[nodiscard]] bool getEnabled(){return _enabled;}
      void setEnabled(bool enabled){_enabled = enabled;}
      void clear(); //remove all children
      std::optional<std::reference_wrapper<std::unique_ptr<TreeItemMeta>>> getMetaData(const std::string& key);
      void setMetaData(const std::string& key, std::unique_ptr<TreeItemMeta> meta);
   protected:
      std::map<std::string, std::unique_ptr<TreeItemMeta>> metaData;
      void setGeneration(size_t generation);
      std::string _text;
      bool _enabled = true; //used to limit interaction and gray it out.
      TreeItem* parent;
      bool isRoot = false;
      bool expanded = true; //unexpanded tree items are visible, it's their children that are not;
      bool visible = true;
      bool expandable = true;
      Tree* _tree;
      std::vector<std::unique_ptr<TreeItem>> children;

   private:
      //the "level" of the branch on which this leaf appears.
      // For example, the root is always generation 0, its children are generation 1,
      // those children's children are generation 2, and so on.
      // Orphaned/null items have generation -1;
      size_t _generation = GENERATION_NULL;

      friend class Tree;
      friend class TreeItemContainerInterface;
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
      EVENT_ARGS(EventItemHovered, 654987984657, TreeItem*& item), item(item){}
         TreeItem*& item;
      };

      [[nodiscard]] std::optional<TreeItem*> getRoot() const {if (root) return root.get(); return {};}
      void setHideRoot(bool hide){_hideRoot = hide; determineVisible();}
      TreeItem* setRoot(std::unique_ptr<TreeItem>&& item);
   //   std::vector<TreeItem*> getItem(const TreePath&) const;

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
      void _on_mouse_exit() override { _hoveredMeta.reset();/*_hoveredRowNum = -1;*/}
      std::optional<TreeItemImplDetails*> getMetaAt(const Pos<float>& localPos);
   private:

      std::unique_ptr<TreeItem> root;
      std::vector<TreeItem*> order; //a full accounting of the tree's contents, used by the iterator.
      std::vector<TreeItemImplDetails*> visible; //a
      bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
      std::optional<TreeItemImplDetails*>_hoveredMeta;

      friend class TreeItem;
   };
}