#include "Tree.h"

using namespace std;

std::optional<std::shared_ptr<TreeItem>> TreeItemContainer::getDescendent(int& index) {
   //query as if the tree is fully expanded. So the first descendent is the root's first child,
   // the second descendent is THAT child's first child, and so on. Only when a child does not have any children
   // do we move to a sibling
   for (auto& child : children){
      index--;
      auto descendent = child->getDescendent(index);
      if (descendent){
         return descendent;
      }
   }
   if (hasDescendent(index)) {
      return children[index];
   }
   //we don't have enough descendents - pop off how many we do have from the index
   index -= children.size();
   return nullopt;
}