#include "Tree.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* TreeItem::push_back(std::unique_ptr<TreeItem>&& newChildItem) {
   children.push_back(std::move(newChildItem));
   auto item = children.back().get();
   item->parent = this;
   //find the root and recalculate the reference vector
   if (_tree) item->_tree = _tree;
   item->setGeneration(_generation+1);
   if (_tree) _tree->determineOrdering();
   return item;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<TreeItem> TreeItem::removeItem(size_t index){
   auto it = getChildren().begin() + index;
   auto ptr = std::move(*it);
   getChildren().erase(it);

   //item no longer is in the tree so has no parent
   ptr->parent = nullptr;

   //let the tree know to recalculate
   if (_tree) _tree->determineOrdering();
   //remove reference to tree
   ptr->_tree = nullptr;
   ptr->_generation = TreeItem::GENERATION_NULL;
   return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineOrdering(){
   order.clear();
   std::function<void(TreeItem*)> pushToVector = [&](TreeItem* item){
      order.push_back(item);
      for (auto& child : item->children){
         pushToVector(child.get());
      }
   };
   pushToVector(root.get());
   determineVisible();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineVisible() {
   //count how many rows are visible/expanded
   visible.clear();
   std::function<void(TreeItem*)> pushVisible = [&](TreeItem* item){
      //root item must not be hidden, otherwise if this item isn't root, then it's parent must be expanded
      if ((item->isRoot && !_hideRoot) || (!item->isRoot)){
         visible.push_back(new TreeItemImplDetails(item, visible.size()));
      }
      //set the rect that contains this item
      Pos<float> startPos = {0, (float)(visible.size() - 1) * ROW_HEIGHT};
      visible.back()->expansionIconClickRegion = {startPos, {getWidth(), ROW_HEIGHT}};
      if (item->expanded) {
         for (auto &child: item->children) {
            pushVisible(child.get());
         }
      }
   };
   if (root) pushVisible(root.get());
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::render2D() const{
   drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary);
   // draw the items
   auto& font = theme->font;
   auto pos = Pos<float>(0,-ROW_HEIGHT);
   long long generationOffset = _hideRoot ? -1 : 0;
   size_t currentRow = 0;
   for (auto it = visible.begin(); it!=visible.end(); it++) {
      auto& itemMeta = *it;
      auto& item = itemMeta->item;
      pos += Pos<float>(0, theme->font.size);

      //highlight the hovered row
      if (_hoveredMeta && _hoveredMeta.value()->visibleRowIndex == currentRow){
         drawRectangle({pos, {getWidth(), theme->font.size}}, COLORS::gray);
      }

      char c = item->expanded ? '-' : '+';

      std::string expansionRegionText = c + std::string(generationOffset + item->_generation, c);
      auto enabledColor = font.color;
      ReyEngine::ColorRGBA disabledColor = {127, 127, 127, 255};
      if (!item->_enabled) {
         font.color = disabledColor;
      }
      drawText(expansionRegionText + item->getText(), pos, font);
      if (!item->_enabled) {
         font.color = enabledColor;
      }
      currentRow++;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Tree::_unhandled_input(const InputEvent& event) {
   switch (event.eventId){
       case InputEventMouseMotion::getUniqueEventId():
       case InputEventMouseButton::getUniqueEventId():
          auto mouseEvent = event.isMouse().value();
          if (!mouseEvent->isInside()) return nullptr;
          auto localPos = mouseEvent->getLocalPos();

          //figure out which row the cursor is in
          auto meta = getMetaAt(localPos);
          if (meta){
             bool newHovered = _hoveredMeta != meta;
             _hoveredMeta = meta;
             //item hover event
             if (newHovered) {
                EventItemHovered itemHoverEvent(this, meta.value()->item);
                publish(itemHoverEvent);
             }
          } else {
             _hoveredMeta.reset();
          }

          //mouse click
          if (event.isEvent<InputEventMouseButton>() && meta){
             auto itemAt = meta.value()->item;
             //expand/shrink branch
             auto btnEvent = event.toEvent<InputEventMouseButton>();
             if (!btnEvent.isDown) {
                Logger::debug() << "click at = " << itemAt->_text << endl;
                if (!itemAt->children.empty() && itemAt->getExpandable()){
                   if (meta.value()->expansionIconClickRegion.contains(localPos)) {
                      itemAt->setExpanded(!itemAt->getExpanded());
                      determineVisible();
                      return this;
                   }
                }
                //publish on item click
                EventItemClicked itemClickedEvent(this, itemAt);
                publish(itemClickedEvent);

             }
          }
          return this;
       }
       return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
TreeItem* Tree::setRoot(std::unique_ptr<TreeItem>&& item) {
    root = std::move(item);
    root->isRoot = true;
    root->_tree = this;
    root->setGeneration(0);
    determineOrdering();
    return root.get();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::TreeItemImplDetails*> Tree::getMetaAt(const Pos<float>& localPos) {
   auto rowHeight = theme->font.size;
   int rowAt = localPos.y / rowHeight;
   if (rowAt < visible.size()) {
      return visible.at(rowAt);
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::setGeneration(size_t generation){
   _generation = generation;
   for (auto& child : getChildren()){
      child->_tree = _tree;
      child->setGeneration(generation + 1);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::clear() {
   children.clear();
   if (_tree) _tree->determineOrdering();
}

///////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::setMetaData(const std::string &key, std::unique_ptr<TreeItemMeta> meta) {
   metaData[key] = std::move(meta);
}
///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::reference_wrapper<std::unique_ptr<TreeItemMeta>>> TreeItem::getMetaData(const std::string &key) {
    auto found = metaData.find(key);
    if (found != metaData.end()){
        return found->second;
    }
    return nullopt;
}

//
///////////////////////////////////////////////////////////////////////////////////////////
//std::vector<std::shared_ptr<TreeItem>> Tree::getItem(const Tree::TreePath& path) const {
//   //traverse the tree and try to find the item in question
//   std::vector<std::shared_ptr<TreeItem>> retval;
//   if (!getRoot()) return retval;
//   auto currentBranch = getRoot().value();
//   for (const auto& element : path.elements()){
//      if (currentBranch->getText() == element){
//         continue;
//      }
//      return retval;
//   }
//}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//Tree::TreePath::TreePath(const std::string &path) {
//   if (path.empty()) return;
//   _tail_elements = string_tools::split(path, TreePath::separator);
//   if (_tail_elements.empty()) return;
//   _head = _tail_elements.back();
//   _tail_elements.pop_back();
//   empty = false;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::string Tree::TreePath::tail() const {
//   return string_tools::join(string(separator, 1), _tail_elements);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::string Tree::TreePath::head() const {
//   return _head;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::string Tree::TreePath::path() const {
//   auto tail = string_tools::join(string(separator, 1), elements());
//   return tail + separator + head();
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::vector<std::string> Tree::TreePath::elements() const {
//   vector<string> retval = _tail_elements;
//   retval.push_back(_head);
//   return retval;
//}