#include "SearchNode.h"

SearchNode::~SearchNode() {
    for (auto reference : references){
        reference.get().removeConnection(*this);
    }
}

void SearchNode::updateParent(SearchNode& _parent, double _cost, double connectionCost) {
   //old cost is greater than new cost - update parent
   if(cost > _cost + baseCost){
      parent = _parent;
      cost = _cost + baseCost * connectionCost;
      calculateCombinedCost();
   }
}

void SearchNode::setHeuristic(double _heuristic) {
   heuristic = _heuristic;
}

void SearchNode::setCost(double _cost, double costConnection) {
   cost = _cost + baseCost * costConnection;
}

void SearchNode::calculateCombinedCost() {
   combinedCost = cost + heuristic;
}

void SearchNode::addConnection(std::reference_wrapper<SearchNode> connection, double cost) {
   //todo: check to see if the connection exists
   connections.insert({connection, cost});
   references.insert(connection);
}

void SearchNode::removeConnection(std::reference_wrapper<SearchNode> connection) {
    connections.erase(connection);
    references.erase(connection);
}