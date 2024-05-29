#include "SearchNode.h"

void SearchNode::updateParent(SearchNode *_parent, float _cost) {
   //old cost is greater than new cost - update parent
   if(cost > _cost){
      parent = _parent;
      cost = _cost;
   }
}