#include "Tree.h"

using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::push_back(std::shared_ptr<TreeItem> &item) {
   children.push_back(item);
   auto me = downcasted_shared_from_this<TreeItem>();
   item->parent = me;
   //find the root and recalculate the reference vector
   item->tree = me->tree;
   item->generation = generation + 1;
   tree->determineOrdering();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<TreeItem> TreeItem::removeItem(size_t index){
   auto ptr = getChildren().at(index);
   auto it = getChildren().begin() + index;
   getChildren().erase(it);
   ptr->parent.reset();

   //let the tree know to recalculate
   tree->determineOrdering();
   //remove reference to tree
   ptr->tree = nullptr;
   ptr->generation = TreeItem::GENERATION_NULL;
   return ptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineOrdering(){
   order.clear();
   std::function<void(std::shared_ptr<TreeItem>&)> pushToVector = [&](std::shared_ptr<TreeItem>& item){
      order.push_back(item);
      for (auto& child : item->children){
         pushToVector(child);
      }
   };
   pushToVector(root);
   determineVisible();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::determineVisible() {
   //count how many rows are visible/expanded
   visible.clear();
   std::function<void(std::shared_ptr<TreeItem>&)> pushVisible = [&](std::shared_ptr<TreeItem>& item){
      //root must not be hidden, otherwise if it isn't root, then it's parent must be expanded
//      Application::printDebug() << "visiting " << item->_text << endl;
      if ((item->isRoot && !_hideRoot) || (!item->isRoot)){
         auto meta = make_shared<TreeItemMeta>(item, visible.size());
         visible.push_back(meta);
      }
      if (item->expanded) {
         for (auto &child: item->children) {
            pushVisible(child);
         }
      }
   };
   pushVisible(root);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::render() const{
   _drawRectangle(getRect().toSizeRect(), getThemeReadOnly().background.colorPrimary.getConst());
   // draw the items
   auto font = getThemeReadOnly().font.value;
   auto pos = ReyEngine::Pos<int>(0,-20);
   long long generationOffset = _hideRoot ? -1 : 0;
   size_t currentRow = 0;
   for (auto it = visible.begin(); it!=visible.end(); it++) {
      auto& itemMeta = *it;
      auto& item = itemMeta->item;
      pos += ReyEngine::Pos<int>(0, getThemeReadOnly().font.getConst().size);

      //highlight the hovered row
      if (_hoveredMeta && _hoveredMeta.value()->visibleRowIndex == currentRow){
         _drawRectangle({pos, {getWidth(), (int) getThemeReadOnly().font.getConst().size}}, COLORS::gray);
      }

      char c = item->expanded ? '-' : '+';

      std::string expansionRegionText = c + std::string(generationOffset + item->generation, c);
      auto enabledColor = font.color;
      ReyEngine::ColorRGBA disabledColor = {127, 127, 127, 255};
      if (!item->_enabled) {
         font.color = disabledColor;
      }
      _drawText(expansionRegionText + item->getText(), pos, font);
      if (!item->_enabled) {
         font.color = enabledColor;
      }
      itemMeta->expansionIconClickRegion = {pos, ReyEngine::measureText(expansionRegionText, font)};
      currentRow++;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Tree::_unhandled_input(InputEvent& event) {
    switch (event.eventId){
       case InputEventMouseMotion::getUniqueEventId():
       case InputEventMouseButton::getUniqueEventId():
          auto mouseEvent = event.toEventType<InputEventMouse>();
          auto localPos = globalToLocal(mouseEvent.globalPos);
          if (!_rect.value.toSizeRect().isInside(localPos)){
             return false;
          }

          //figure out which row the cursor is in
          auto meta = getMetaAt(localPos);
          if (meta){
             _hoveredMeta = meta;
             //item hover event
             ItemHoverEvent itemHoverEvent(toEventPublisher(), meta.value()->item);
             publish(itemHoverEvent);
          } else {
             _hoveredMeta.reset();
          }

          //mouse click
          if (event.isEvent<InputEventMouseButton>() && meta){
             auto itemAt = meta.value()->item;
             //expand/shrink branch
             auto btnEvent = event.toEventType<InputEventMouseButton>();
             if (!btnEvent.isDown) {
//                Application::printDebug() << "click at = " << itemAt->_text << endl;
                if (!itemAt->children.empty() && itemAt->getExpandable()){
                   if (meta.value()->expansionIconClickRegion.isInside(localPos)) {
                      itemAt->setExpanded(!itemAt->getExpanded());
                      determineVisible();
                      return true;
                   }
                }
                //publish on item click
                ItemClickedEvent itemClickedEvent(toEventPublisher(), itemAt);
                publish(itemClickedEvent);

             }
          }
          return true;
       }
       return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::setRoot(std::shared_ptr<TreeItem> item) {
    root = item;
    order.clear();
    order.push_back(root);
    root->isRoot = true;
    root->tree = this;
    root->generation = 0;
    determineOrdering();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Tree::TreeItemMeta>> Tree::getMetaAt(const ReyEngine::Pos<int>& localPos) {
   auto rowHeight = getThemeReadOnly().font.getConst().size;
   int rowAt = localPos.y / rowHeight;
   if (rowAt < visible.size()) {
      return visible.at(rowAt);
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