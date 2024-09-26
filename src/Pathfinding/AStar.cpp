#include "AStar.h"
#include <cmath>

using namespace ReyEngine;
using namespace std;

void visitNode(GraphNode& graphNode, const GraphNode& goal, optional<std::reference_wrapper<SearchNode>> parent){
   cout << "Visiting open node " << graphNode._coord << endl;
   auto& searchNode = graphNode.getSearchNode();
   searchNode.setHeuristic(graphNode.getHeuristic(goal));
   searchNode.setParent(parent);
   searchNode.addCost(graphNode._cost);
}

std::optional<std::reference_wrapper<SearchNode>> AStar::findPath(GraphNode &graphStart, GraphNode& graphGoal) {
   std::reference_wrapper<SearchNode> currentNode = graphStart.getSearchNode();
   graphStart.getSearchNode().reset();
   visitNode(graphStart, graphGoal, {});
   while(currentNode.get().getCoords() != graphGoal.getCoords()){
      auto index = getIndex(currentNode.get()._coord);
      queryGraph(_graph.at(index), graphGoal);
      if(_frontier.empty()) return {};
      currentNode = _frontier.top();
      _frontier.pop();
   }
   //we found the goal
   return graphGoal.getSearchNode();
}

void AStar::queryGraph(GraphNode& openNode, const GraphNode& goal) {
   for (auto& [nodeCost, _connectedNode] : openNode._connections){
      auto& connectedNode = _connectedNode.get();
      auto found = _visitedNodes.find(connectedNode);
      if(found != _visitedNodes.end()){
         auto& existingNode = found->get();
         //Found the node in the visited nodes so need to update the parent
         visitNode(existingNode, goal, openNode.getSearchNode());
      } else {
         //add this node to the visited set
         auto& searchNode = connectedNode.getSearchNode();
         visitNode(connectedNode, goal, searchNode);
         _frontier.emplace(searchNode);
         _visitedNodes.insert(openNode);
      }
   }
}