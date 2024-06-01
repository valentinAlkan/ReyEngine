#include "AStar.h"
#include <cmath>

bool AStar::findPath(SearchNode *start, SearchNode *goal) {
   _start = start;
   _goal = goal;
}

void AStar::expandNode(SearchNode *node) {
   for(auto it = node->connections.begin(); it != node->connections.end(); it++){
      auto openIt = _openSet.find(it->first->id);
      if(openIt != _openSet.end()){
         //Found the node in the openSet so need to update the parent
         openIt->second->updateParent(node, node->cost);
      } else {
         //todo: create function to call all three in one line
         it->first->setCost(node->cost, it->second);
         it->first->setHeuristic(calculateHeuristic(it->first));
         it->first->calculateCombinedCost();
         _expandedNodes.push(it->first);
         _openSet.insert({it->first->id, it->first});
      }
   }
}

float AStar::calculateHeuristic(SearchNode *node) {
   float xTerm = (_goal->)
}