#include "AStar.h"
#include <cmath>

bool AStar::findPath(SearchNode& start, SearchNode& goal) {
   _goal = goal;
   SearchNode& currentNode = start;
   start.setHeuristic(calculateHeuristic(start, goal));
   start.calculateCombinedCost();
   start.parent = {};
   while(currentNode != _goal){
      expandNode(currentNode);
      if(_frontier.empty()) return false;
      currentNode = _frontier.top();
      _frontier.pop();
   }
   return true;
}

void AStar::expandNode(SearchNode& openNode) {
   for (auto& [_connectedNode, pathCost] : openNode.connections){
      auto& connectedNode = _connectedNode.get();
      auto found = _visitedNodes.find(_connectedNode);
      if(found != _visitedNodes.end()){
         auto& existingNode = found->get();
         //Found the node in the openSet/visited nodes so need to update the parent
         existingNode.updateParent(openNode, openNode.cost, pathCost);
      } else {
         connectedNode.setCost(openNode.cost, pathCost);
         connectedNode.setHeuristic(calculateHeuristic(openNode, _goal->get()));
         connectedNode.calculateCombinedCost();
         connectedNode.parent = openNode;
         _frontier.emplace(connectedNode);
         _visitedNodes.insert(openNode);
      }
   }
}