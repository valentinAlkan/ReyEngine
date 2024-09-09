#include "SearchNode.h"

SearchNode::~SearchNode() {
    for (auto reference : references){
        reference->removeConnection(shared_from_this());
    }
}

void SearchNode::updateParent(std::shared_ptr<SearchNode> _parent, float _cost, float connectionCost) {
   //old cost is greater than new cost - update parent
   if(cost > _cost + baseCost){
      parent = _parent;
      cost = _cost + baseCost * connectionCost;
      calculateCombinedCost();
   }
}

void SearchNode::setHeuristic(float _heuristic) {
   heuristic = _heuristic;
}

void SearchNode::setCost(float _cost, float costConnection) {
   cost = _cost + baseCost * costConnection;
}

void SearchNode::calculateCombinedCost() {
   combinedCost = cost + heuristic;
}

void SearchNode::addConnection(float cost, std::shared_ptr<SearchNode> connection) {
   //todo: check to see if the connection exists
   connections.insert({connection, cost});
   references.insert(connection);
}

void SearchNode::removeConnection(std::shared_ptr<SearchNode> connection) {
    connections.erase(connection);
    references.erase(connection);
}