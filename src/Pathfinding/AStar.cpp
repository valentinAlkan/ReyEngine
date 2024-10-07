#include "AStar.h"
#include <cmath>

using namespace ReyEngine;
using namespace std;

/*
OPEN // the set of nodes to be evaluated
CLOSED // the set of nodes already evaluated
add the start node to OPEN

loop
	current = node in OPEN with the lowest f_cost
	remove current from OPEN
	add current to CLOSED

	if current is the target node //path has been found
		return

	foreach neighbor of the current node
		if neighbor is not traversable or neighbour is in CLOSED
			skip to the next neighbor

		if new path to neighbor is shorter or neighbor is not in OPEN
			set f_cost of neighbor
			set parent of neighbor to current
			if neighbor is not in OPEN
				add neighbor to OPEN
 */

std::optional<std::reference_wrapper<GraphNode>> AStar::findPath(GraphNode& graphStart, GraphNode& graphGoal) {
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
         bool inOpen = foundOpen != _openSet.end();
         //expand (or re-expand if shorter) this node
         if (!inOpen || connectedNode->_fcost < currentNode->_fcost){
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