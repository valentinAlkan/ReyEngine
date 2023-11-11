#pragma once

#include <utility>

#include "Control.hpp"
#include "IndexedMap.h"
#include "Layout.hpp"

class Tree;
class TreeItem;
class TreeItemContainer;

class TreeItemBase{
   bool isRoot = false;
   bool isEnd = false;
//   virtual TreeItemBase& getNext() = 0;
};


class TreeItemIterator{
public:
   TreeItemBase& operator*(){return value;}
protected:
   explicit TreeItemIterator(TreeItemBase& child, TreeItemBase& next): value(child), next(next){};
   TreeItemBase& value;
   TreeItemBase& next;
};

class TreeItemContainer : private TreeItemBase {
public:
   /////////////////////////////////////////////////////////////////////////////////////////
   void pushBack(std::shared_ptr<TreeItem>& item){
      children.push_back(item);
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   void insertItem(int index, std::shared_ptr<TreeItem>& item){
      children.insert(children.begin()+index, item);
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   std::shared_ptr<TreeItem> removeItem(int index){
      auto ptr = children.at(index);
      auto it = children.begin() + index;
      children.erase(it);
      return ptr;
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   void sort(std::function<bool(const std::shared_ptr<TreeItem>& a, const std::shared_ptr<TreeItem>& b)>& fxLessthan){
      std::sort(children.begin(), children.end(), fxLessthan);
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   std::vector<std::shared_ptr<TreeItem>>& getChildren() {return children;}
   /////////////////////////////////////////////////////////////////////////////////////////



public:
   struct Iterator{
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = TreeItem;
      using pointer           = std::shared_ptr<TreeItem>;
      using reference         = TreeItem&;

      Iterator(pointer ptr) : m_ptr(ptr) {}

      reference operator*() const { return *m_ptr.get(); }
      pointer operator->() { return m_ptr; }
      Iterator& operator++() {

         return *this;
      }
      Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
      friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
      friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

   private:
      pointer m_ptr;
      int currentChild = 0;
   };

   Iterator begin() { return {children[0]}; }
   Iterator end()   { return {invalid}; }
protected:
   std::vector<std::shared_ptr<TreeItem>> children;
   std::shared_ptr<TreeItem> invalid;

   std::optional<std::shared_ptr<TreeItem>> getDescendent(int& index);

   //travel through the tree and get descendent at position <index>
   bool hasDescendent(int index){return children.size() > index;}
};


class TreeItem : public EventSubscriber, public TreeItemContainer{
public:
   TreeItem(const std::string& text=""): _text(text){}
   void setText(const std::string& text){_text = text;}
   std::string getText(){return _text;}
protected:
   std::string _text;
   std::vector<std::unique_ptr<TreeItem>> children;
};

class Tree : public VLayout, public TreeItemContainer {
GFCSDRAW_OBJECT(Tree, VLayout){}
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

protected:
   void render() const override{
      _drawRectangle(getRect().toSizeRect(), COLORS::red);
//      draw the items
      auto font = getThemeReadOnly().font.value;
//      for (const auto& item : _data){
//         _drawText(item)
//      }
   }

private:
//   IndexedMap<size_t, std::unique_ptr<TreeItem>> _map; //todo: gotta go fast?

};