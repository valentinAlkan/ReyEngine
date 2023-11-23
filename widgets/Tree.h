#pragma once
#include "Layout.hpp"

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

class TreeItem: public TreeItemContainerInterface, public inheritable_enable_shared_from_this<TreeItem> {
public:
   static constexpr long long GENERATION_NULL = -1;
   TreeItem(const std::string& text=""): _text(text){}
   void setText(const std::string& text){_text = text;}
   std::string getText() const {return _text;}
   void push_back(std::shared_ptr<TreeItem>& item) override;
   std::shared_ptr<TreeItem> removeItem(size_t index) override;
   std::vector<std::shared_ptr<TreeItem>>& getChildren() override {return children;}
   bool getExpanded(){return expanded;}
   void setExpanded(bool _expanded){expanded = _expanded;}
   bool getExpandable(){return expandable;}
   void setExpandable(bool _expandable){expandable = _expandable;}
protected:
   std::string _text;
   std::weak_ptr<TreeItem> parent;
   bool isRoot = false;
   bool expanded = true; //unexpanded tree items are visible, it's their children that are not;
   bool visible = true;
   bool expandable = true;
   Tree* tree = nullptr;
   std::vector<std::shared_ptr<TreeItem>> children;

private:
   //the "level" of the branch on which this leaf appears.
   // For example, the root is always generation 0, its children are generation 1,
   // those children's children are generation 2, and so on.
   // Orphaned items have generation -1;
   long long generation = GENERATION_NULL;

   friend class Tree;
   friend class TreeItemContainerInterface;
};

class Tree : public VLayout {
   GFCSDRAW_OBJECT(Tree, VLayout){
      acceptsHover=true;
   }
public:
   ///! not sure if this is necessary, why do we need to index tree items by a string? The
   /// tree is a gui element, intended to be clicked on, so anyone can get an element by
   /// clicking on it or traversing it from the root based on some other criteria.
   /// Leaving here for now just in case, but I dont think its necessary.
//   struct TreePath{
//      static constexpr char separator = '/';
//      TreePath(const std::string& path);
//      std::string head() const;
//      std::string tail() const;
//      std::string path() const;
//      std::vector<std::string> elements() const;
//   protected:
//      std::vector<std::string> _tail_elements;
//      std::string _head;
//      bool empty = true;
//   };
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
      EVENT_CTOR_SIMPLE(TreeClickEvent, Event<TreeClickEvent>, GFCSDraw::Pos<int> localPos), localPos(localPos){}
      GFCSDraw::Pos<int> localPos;
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
         auto& order = root->tree->order;
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
   void determineOrdering();
   void determineVisible();
   void render() const override;
   virtual Handled _unhandled_input(InputEvent&);
   virtual void _on_mouse_enter(){};
   virtual void _on_mouse_exit(){ _hoveredRowNum = -1;}
private:

   //Stores extra details that the tree can use
   struct TreeItemMeta{
      TreeItemMeta(std::shared_ptr<TreeItem> item): item(item){}
      GFCSDraw::Rect<int> expansionIconClickRegion; //where we can click to determine if an item should be "expanded" or not;
      std::shared_ptr<TreeItem> item;
   };

   std::shared_ptr<TreeItem> root;
   std::vector<std::shared_ptr<TreeItem>> order;
   std::vector<std::shared_ptr<TreeItemMeta>> visible;
   bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
   size_t _hoveredRowNum = -1; //the row number currently being hovered

   friend class TreeItem;
};