#include "Tree.h"

using namespace std;

Tree::Tree(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float> &r, std::shared_ptr<TreeItem> &root)
: VLayout(name, typeName, r)
, root(root)
{
   root->isRoot = true;
   order.push_back(root);
   root->tree = this;
   root->generation = 0;
}

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
         visible.push_back(item);
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
   _drawRectangle(getRect().toSizeRect(), COLORS::red);
   // draw the items
   auto font = getThemeReadOnly().font.value;
   auto pos = GFCSDraw::Pos<int>(0,-20);
   long long generationOffset = _hideRoot ? -1 : 0;
   size_t currentRow = 0;
   for (auto it = visible.begin(); it!=visible.end(); it++) {
      auto& item = *it;
      pos += GFCSDraw::Pos<int>(0, getThemeReadOnly().font.get().size);

      //highlight the hovered row
      if (_hoveredRowNum == currentRow){
         _drawRectangle({pos, {getWidth(), (int)getThemeReadOnly().font.get().size}}, COLORS::gray);
      }

      char c = item->expanded ? '-' : '+';
      _drawText(c + std::string(generationOffset + item->generation, c) + item->getText(), pos, font);
      currentRow++;
   }

   _drawRectangle({testPos, {20,20}}, COLORS::blue);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Tree::_unhandled_input(InputEvent& event) {
    switch (event.eventId){
       case InputEventMouseMotion::getUniqueEventId():
       case InputEventMouseButton::getUniqueEventId():
          auto mouseEvent = event.toEventType<InputEventMouse>();
          auto posLocal = globalToLocal(mouseEvent.globalPos);
          testPos = posLocal;

          //figure out which row the cursor is in
          auto rowHeight = getThemeReadOnly().font.get().size;
          int rowAt = posLocal.y / rowHeight;
          std::shared_ptr<TreeItem> itemAt;
          if (rowAt < visible.size()) {
             _hoveredRowNum = rowAt;
             itemAt = visible.at(rowAt);
//             Application::printDebug() << "item at = " << itemAt->_text << endl;
          }

          //mouse click
          if (event.isEvent<InputEventMouseButton>() && itemAt){
             //expand/shrink branch
             auto btnEvent = event.toEventType<InputEventMouseButton>();
             if (!btnEvent.isDown) {
                Application::printDebug() << "click at = " << itemAt->_text << endl;
                if (!itemAt->children.empty()) {
                   itemAt->setExpanded(!itemAt->getExpanded());
                   determineVisible();
                }
             }
          }

          return true;
       }
}