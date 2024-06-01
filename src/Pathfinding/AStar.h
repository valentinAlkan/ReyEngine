#pragma once
#include "SearchNode.h"

//class to preform an AStar algorithm
struct coord{
   int x, y;
};
class AStar {
public:
   AStar(){};

   /**
    * Finds the best path from the start node to the end node using an A* algorithm.
    * The path can then be read by reading the parent of the _goal node and tracing the parents to the start
    * @param _start : The node to start the path
    * @param _goal : The goal node of the path; Read the parent of this node up the tree to the start
    * @return : true if a path was found; false if no path was found;
    */
   bool findPath(std::shared_ptr<SearchNode> start, std::shared_ptr<SearchNode> goal);
private:

   std::priority_queue<std::shared_ptr<SearchNode>, std::vector<std::shared_ptr<SearchNode>>, std::greater<std::shared_ptr<SearchNode>>> _expandedNodes;
   std::map<int, std::shared_ptr<SearchNode>> _openSet;

   std::shared_ptr<SearchNode> _start;
   std::shared_ptr<SearchNode> _goal;

   /**
    * places all of the connections in _expandedNodes;
    * Checks to see if the node already exists in _openSet and calls updateParent if so
    * @param node : The node to expand
    */
   void expandNode(std::shared_ptr<SearchNode> node);

   /**
    * calculates the Heuristic of the node with the distance formula
    * @param node : node to calculate for
    * @return : the heuristic
    */
   float calculateHeuristic(std::shared_ptr<SearchNode> node);
};
