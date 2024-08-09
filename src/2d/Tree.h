#pragma once
#include "Layout.h"

namespace ReyEngine{
   class Tree;
   class TreeItem;
   //todo: this doesn't need to be an interface since tree doesn't do it anymore. move all this to tree item
   class TreeItemContainerInterface {
   public:
      virtual void push_back(std::shared_ptr<TreeItem>& item) = 0;
      /////////////////////////////////////////////////////////////////////////////////////////
   //   void insertItem(int index, std::shared_ptr<TreeItem>& item){getChildren().insert(getChildren().begin()+index, item);}
      /////////////////////////////////////////////////////////////////////////////////////////
      virtual std::shared_ptr<TreeItem> removeItem(size_t index) = 0;
      void sort(std::function<bool(const std::shared_ptr<TreeItem>& a, const std::shared_ptr<TreeItem>& b)>& fxLessthan){
         std::sort(getChildren().begin(), getChildren().end(), fxLessthan);
      }
      /////////////////////////////////////////////////////////////////////////////////////////
   protected:
      virtual std::vector<std::shared_ptr<TreeItem>>& getChildren() = 0;
   };

   //A struct to hold extra data for us
   struct TreeItemMeta {};
   class TreeItem: public TreeItemContainerInterface, public inheritable_enable_shared_from_this<TreeItem> {
   public:
      static constexpr long long GENERATION_NULL = -1;
      TreeItem(const std::string& text=""): _text(text){}
      void setText(const std::string& text){_text = text;}
      std::string getText() const {return _text;}
      void push_back(std::shared_ptr<TreeItem>& newChildItem) override;
      std::shared_ptr<TreeItem> removeItem(size_t index) override;
      std::vector<std::shared_ptr<TreeItem>>& getChildren() override {return children;}
      bool getExpanded(){return expanded;}
      void setExpanded(bool _expanded){expanded = _expanded;}
      bool getExpandable(){return expandable;}
      void setExpandable(bool _expandable){expandable = _expandable;}
      bool getEnabled(){return _enabled;}
      void setEnabled(bool enabled){_enabled = enabled;}
      void clear(); //remove all children
      std::optional<std::reference_wrapper<std::unique_ptr<TreeItemMeta>>> getMetaData(const std::string& key);
      void setMetaData(const std::string& key, std::unique_ptr<TreeItemMeta> meta);
   protected:
      std::map<std::string, std::unique_ptr<TreeItemMeta>> metaData;
      void setGeneration(long long generation);
      std::string _text;
      bool _enabled = true; //used to limit interaction and gray it out.
      std::weak_ptr<TreeItem> parent;
      bool isRoot = false;
      bool expanded = true; //unexpanded tree items are visible, it's their children that are not;
      bool visible = true;
      bool expandable = true;
      std::weak_ptr<Tree> _tree;
      std::vector<std::shared_ptr<TreeItem>> children;

   private:
      //the "level" of the branch on which this leaf appears.
      // For example, the root is always generation 0, its children are generation 1,
      // those children's children are generation 2, and so on.
      // Orphaned items have generation -1;
      long long _generation = GENERATION_NULL;

      friend class Tree;
      friend class TreeItemContainerInterface;
   };

   class Tree : public VLayout {
      REYENGINE_OBJECT_BUILD_ONLY(Tree, VLayout){
         acceptsHover=true;
      }
   public:
      REYENGINE_DEFAULT_BUILD(Tree);
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      struct ItemAddedEvent : public Event<ItemAddedEvent> {
         EVENT_CTOR_SIMPLE(ItemAddedEvent, Event<ItemAddedEvent>, const std::shared_ptr<TreeItem>& item), item(item){}
         const std::shared_ptr<TreeItem>& item;
      };
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      struct ItemRemovedEvent : public Event<ItemRemovedEvent> {
         EVENT_CTOR_SIMPLE(ItemRemovedEvent, Event<ItemRemovedEvent>, std::shared_ptr<TreeItem>& item), item(item){}
         std::shared_ptr<TreeItem> item;
      };
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      struct ItemClickedEvent : public Event<ItemClickedEvent> {
         EVENT_CTOR_SIMPLE(ItemClickedEvent, Event<ItemClickedEvent>, std::shared_ptr<TreeItem>& item), item(item){}
         std::shared_ptr<TreeItem> item;
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      struct ItemHoverEvent : public Event<ItemHoverEvent> {
         EVENT_CTOR_SIMPLE(ItemHoverEvent, Event<ItemHoverEvent>, std::shared_ptr<TreeItem>& item), item(item){}
         std::shared_ptr<TreeItem> item;
      };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      struct TreeClickEvent : public Event<TreeClickEvent> {
         EVENT_CTOR_SIMPLE(TreeClickEvent, Event<TreeClickEvent>, ReyEngine::Pos<int> localPos), localPos(localPos){}
         ReyEngine::Pos<int> localPos;
      };

      std::optional<std::shared_ptr<TreeItem>> getRoot() const {if (root) return root; return {};}
      void setHideRoot(bool hide){_hideRoot = hide; determineVisible();}
      void setRoot(std::shared_ptr<TreeItem> item);
   //   std::vector<std::shared_ptr<TreeItem>> getItem(const TreePath&) const;

      struct Iterator{
         using iterator_category = std::forward_iterator_tag;
         using difference_type   = std::ptrdiff_t;
         using value_type        = TreeItem;
         using pointer           = std::shared_ptr<TreeItem>;
         using reference         = TreeItem&;

         Iterator(pointer ptr) : iterPtr(ptr), root(ptr) {}

         reference operator*() const { return *iterPtr.get(); }
         pointer operator->() { return iterPtr; }
         Iterator& operator++() {
            auto& order = root->_tree.lock()->order;
            if (leafIndex >= order.size()){
               iterPtr = nullptr;
            } else {
               iterPtr = order[leafIndex++];
            }
            return *this;
         }
         Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
         friend bool operator== (const Iterator& a, const Iterator& b) { return a.iterPtr == b.iterPtr; };
         friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iterPtr != b.iterPtr; };

      private:
         pointer iterPtr;
         size_t leafIndex = 1;
         std::shared_ptr<TreeItem> root;
      };
      Iterator begin() { return {root}; }
      Iterator end()   { return {nullptr}; }

   protected:

      //Stores extra details that the tree can use
      struct TreeItemMeta{
         TreeItemMeta(std::shared_ptr<TreeItem> item, int visibleRowindex): item(item), visibleRowIndex(visibleRowindex){}
         ReyEngine::Rect<int> expansionIconClickRegion; //where we can click to determine if an item should be "expanded" or not;
         std::shared_ptr<TreeItem> item;
         const int visibleRowIndex = -1;
      };

      void determineOrdering();
      void determineVisible();
      void render() const override;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void _on_mouse_enter() override {};
      void _on_mouse_exit() override { _hoveredMeta.reset();/*_hoveredRowNum = -1;*/}
      std::optional<std::shared_ptr<TreeItemMeta>> getMetaAt(const ReyEngine::Pos<int>& localPos);
   private:

      std::shared_ptr<TreeItem> root;
      std::vector<std::shared_ptr<TreeItem>> order;
      std::vector<std::shared_ptr<TreeItemMeta>> visible;
      bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
   //   size_t _hoveredRowNum = -1; //the row number currently being hovered
      std::optional<std::shared_ptr<TreeItemMeta>>_hoveredMeta;

      friend class TreeItem;
   };
}