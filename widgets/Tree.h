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
protected:
   std::string _text;
   std::weak_ptr<TreeItem> parent;
   bool isRoot = false;
   bool expanded = true; //unexpanded tree items are visible, it's their children that are not;
   bool visible = true;
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
public:
   GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(Tree)
public:
   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
      throw std::runtime_error("Need to implement tree item deserializing");
      auto root = std::make_shared<TreeItem>("root");
      auto retval = std::make_shared<Tree>(instanceName, r, root);
      retval->BaseWidget::_deserialize(properties);
      return retval;
   }
   Tree(const std::string &name, const GFCSDraw::Rect<float> &r, std::shared_ptr<TreeItem>& root) : Tree(name, _get_static_constexpr_typename(), r, root){}
protected:
   void _register_parent_properties() override{
      VLayout::_register_parent_properties();
      VLayout::registerProperties();
   }
   Tree(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float> &r, std::shared_ptr<TreeItem>& root);

public:
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

   std::shared_ptr<TreeItem> getRoot(){return root;}
   void setHideRoot(bool hide){_hideRoot = hide; determineVisible();}

public:
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
   std::shared_ptr<TreeItem> root;
   std::vector<std::shared_ptr<TreeItem>> order;
   std::vector<std::shared_ptr<TreeItem>> visible;
   bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
   size_t _hoveredRowNum = -1; //the row number currently being hovered

   GFCSDraw::Pos<int> testPos;
   friend class TreeItem;
};