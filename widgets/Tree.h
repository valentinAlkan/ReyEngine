#pragma once

#include <utility>

#include "Control.hpp"
#include "IndexedMap.h"
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
   TreeItem(const std::string& text=""): _text(text){}
   void setText(const std::string& text){_text = text;}
   std::string getText() const {return _text;}
   void push_back(std::shared_ptr<TreeItem>& item) override;
   std::shared_ptr<TreeItem> removeItem(size_t index) override;
   std::vector<std::shared_ptr<TreeItem>>& getChildren() override {return children;}
protected:
   std::string _text;
   std::weak_ptr<TreeItem> parent;
   bool isRoot = false;
   Tree* tree = nullptr;
   std::vector<std::shared_ptr<TreeItem>> children;

private:
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
   void _register_parent_properties()
   override{
      VLayout::_register_parent_properties();
      VLayout::registerProperties();
   }
   Tree(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float> &r, std::shared_ptr<TreeItem>& root)
   : VLayout(name, typeName, r)
   , root(root)
   {
      root->isRoot = true;
      order.push_back(root);
      root->tree = this;
   }
//   root = std::make_shared<TreeItem>("Root");

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
            iterPtr = order[leafIndex++].lock();
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
   void determineOrdering(){
      order.clear();
      std::function<void(std::shared_ptr<TreeItem>&)> pushToVector = [&](std::shared_ptr<TreeItem>& item){
         order.push_back(item);
         for (auto& child : item->children){
            pushToVector(child);
         }
      };
      pushToVector(root);
   }
   void render() const override{
      _drawRectangle(getRect().toSizeRect(), COLORS::red);
//      draw the items
      auto font = getThemeReadOnly().font.value;
//      for (const auto& item : _data){
//         _drawText(item)
//      }
   }

private:
   std::shared_ptr<TreeItem> root;
   std::vector<std::weak_ptr<TreeItem>> order;
   bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
   friend class TreeItem;
};