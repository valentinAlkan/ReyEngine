#include "AStar.h"
#include <cmath>

bool AStar::findPath(std::shared_ptr<SearchNode> start, std::shared_ptr<SearchNode> goal) {
   _start = start;
   _goal = goal;
   std::shared_ptr<SearchNode> currentNode = _start;
   _start->setHeuristic(calculateHeuristic(_start));
   _start->calculateCombinedCost();
   _start->parent = nullptr;
   while(currentNode != _goal){
      expandNode(currentNode);
      if(_expandedNodes.empty()) return false;
      currentNode = _expandedNodes.top();
      _expandedNodes.pop();
   }
   return true;
}

void AStar::expandNode(std::shared_ptr<SearchNode> node) {
   for(auto it = node->connections.begin(); it != node->connections.end(); it++){
      auto openIt = _openSet.find(it->first->id);
      if(openIt != _openSet.end()){
         //Found the node in the openSet so need to update the parent
         openIt->second->updateParent(node, node->cost, it->second);
      } else {
         //todo: create function to call all these in one line
         it->first->setCost(node->cost, it->second);
         it->first->setHeuristic(calculateHeuristic(it->first));
         it->first->calculateCombinedCost();
         it->first->parent = node;
         _expandedNodes.push(it->first);
         _openSet.insert({it->first->id, it->first});
      }
   }
}

float AStar::calculateHeuristic(std::shared_ptr<SearchNode> node) {
   float xTerm = (_goal->x_coord - node->x_coord) * (_goal->x_coord - node->x_coord);
   float yTerm = (_goal->y_coord - node->y_coord) * (_goal->y_coord - node->y_coord);
   return sqrt(xTerm + yTerm);
}