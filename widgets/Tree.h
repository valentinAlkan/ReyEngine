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
   void insertItem(int index, std::shared_ptr<TreeItem>& item){getChildren().insert(getChildren().begin()+index, item);}
   /////////////////////////////////////////////////////////////////////////////////////////
   std::shared_ptr<TreeItem> removeItem(int index){
      auto ptr = getChildren().at(index);
      auto it = getChildren().begin() + index;
      getChildren().erase(it);
      return ptr;
   }
   /////////////////////////////////////////////////////////////////////////////////////////
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
   void push_back(std::shared_ptr<TreeItem>& item) override {
      children.push_back(item);
      item->parent = downcasted_shared_from_this<TreeItem>();
   }
protected:
   std::string _text;
   std::weak_ptr<TreeItem> parent;
   std::optional<std::shared_ptr<TreeItem>> getDescendent(size_t& index);
   //travel through the tree and get descendent at position <index>
   std::optional<std::shared_ptr<TreeItem>> getNext(size_t nextIndex){
      auto optDescendent = getDescendent(nextIndex);
      if (optDescendent){
         return optDescendent.value();
      }
      // we have run out of descendents, so we need to travel up the tree
      if (parent.expired()){
         //something is wrong
         throw std::runtime_error("Invalid TreeItem iterator! Parent does not exist!");
      }
      auto _parent = parent.lock();
      //determine what is the index of this in the parent
      size_t siblingIndex = 0;
      for (auto it = _parent->getChildren().begin(); it != _parent->getChildren().end(); it++){
         siblingIndex++;
         auto sibling = *it;
         if (sibling == downcasted_shared_from_this<TreeItem>()){
            //this is us
            break;
         }
      }
      //we now know our own index in our parent's children array. First verify if we have a sibling.
      if (_parent->getChildren().size() <= siblingIndex){
         //there is nowhere else to go
         return std::nullopt;
      }
      // See if our next sibling has a descendent;
      auto sibling = _parent->getChildren().at(siblingIndex);
      //do the same query as before
      return sibling->getNext(nextIndex);
   }
protected:
   /////////////////////////////////////////////////////////////////////////////////////////
   std::vector<std::shared_ptr<TreeItem>>& getChildren() override {return children;}
   std::vector<std::shared_ptr<TreeItem>> children;
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
   {}
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

      Iterator(pointer ptr) : m_ptr(ptr) {}

      reference operator*() const { return *m_ptr.get(); }
      pointer operator->() { return m_ptr; }
      Iterator& operator++() {
         auto optNext = m_ptr->getNext(1);
         if (!optNext){
            m_ptr = nullptr;
         } else {
            m_ptr = optNext.value();
         }
         return *this;
      }
      Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
      friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
      friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

   private:
      pointer m_ptr;
   };

   Iterator begin() { return {root}; }
   Iterator end()   { return {nullptr}; }




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
   std::shared_ptr<TreeItem> root;
   bool _hideRoot = false; //if true, the root is hidden and we can appear as a "flat" tree.
};