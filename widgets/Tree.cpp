#include "Tree.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void TreeItem::push_back(std::shared_ptr<TreeItem> &item) {
   children.push_back(item);
   auto me = downcasted_shared_from_this<TreeItem>();
   item->parent = me;
   //find the root and recalculate the reference vector
   item->tree = me->tree;
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
   return ptr;
}