#include "AStar.h"
#include <cmath>

using namespace ReyEngine;
using namespace PathFinding;
using namespace std;

AStar::AStar(unsigned int sizeX, unsigned int sizeY)
: _sizeX(sizeX)
, _sizeY(sizeY)
{
   for (int x=0; x<sizeX; x++){
      for (int y=0; y<sizeY; y++){
         const auto& coord = Vec2<int>(x,y);
         auto node = std::make_unique<GraphNode>(coord);
         _graph[getIndex(coord)] = std::move(node);
      }
   }
}


std::optional<std::reference_wrapper<AStar::GraphNode>> AStar::findPath(GraphNode& graphStart, GraphNode& graphGoal) {
   auto now = std::chrono::steady_clock::now();
   GraphNode* currentNode = &graphStart;
   _openSet.clear();
   _closedSet.clear();
   _openSet.emplace(&graphStart);
   graphStart._parent = nullptr;
   graphGoal._parent = nullptr;
   while(currentNode != &graphGoal){
      for (auto& connection : currentNode->getConnections()) {
         auto &connectedNode = connection->b;
         auto foundClosed = _closedSet.find(connectedNode);
         if (foundClosed != _closedSet.end()) {
            continue;
         }
         auto foundOpen = _openSet.find(connectedNode);
         //expand (or re-expand if shorter) this node
         if (connectedNode->_fcost < currentNode->_fcost || foundOpen == _openSet.end()){
            expandNode(*connection, currentNode, graphGoal);
         }
         _openSet.emplace(connectedNode);

      }
      if(_openSet.empty()) return {};
      _closedSet.emplace(currentNode);
      auto it = getNodeLowestFCost();
      currentNode = *it;
      _openSet.erase(it);
   }
   //we found the goal
   cout << "goal found in " << chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - now).count() << "us" << endl;
   return graphGoal;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Vec2<int>> AStar::extractPath(const GraphNode& goal) {
   if (!goal._parent) return {};
   std::vector<Vec2<int>> retval;
   auto node = &goal;
   while(node){
      retval.push_back(node->_parent->_coord);
      node = node->_parent;
   }
   std::reverse(retval.begin(), retval.end());
   return retval;
}

std::vector<std::reference_wrapper<AStar::GraphNode>> AStar::getNeighbors(const Vec2<int> &pos, bool includeCorners) {
   std::vector<std::reference_wrapper<GraphNode>> retval;
   std::optional<std::reference_wrapper<GraphNode>> node;
   for (auto _x : {-1,0,1}){
      for (auto _y : {-1,0,1}){
         node = at({pos.x + _x, pos.y + _y});
         if (node && node.value().get()._coord != pos){
            retval.push_back(node.value());
         }
      }
   }
   return retval;
}