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
   bool findPath(SearchNode& start, SearchNode& goal);

private:
    struct SearchNodeComparator {
        bool operator()(const std::reference_wrapper<SearchNode>& lhs, const std::reference_wrapper<SearchNode>& rhs) const {
            return lhs.get() < rhs.get();
        }
    };

    struct CombinedCostComparator{
        bool operator()(const std::reference_wrapper<SearchNode>& a, const std::reference_wrapper<SearchNode>& b) const{
            return a.get().combinedCost > b.get().combinedCost;
        }
    };
   /// The frontier represents the fog of war - that is to say, it should always return the most promising connected node that
   /// has yet to be visited
   std::priority_queue<std::reference_wrapper<SearchNode>, std::vector<std::reference_wrapper<SearchNode>>, CombinedCostComparator> _frontier;
   /// The set of nodes that have already been visited and searched for connections
   std::set<std::reference_wrapper<SearchNode>, SearchNodeComparator> _visitedNodes;
   std::optional<std::reference_wrapper<SearchNode>> _goal; //where we're headed

   /**
    * places all of the connections in _expandedNodes;
    * Checks to see if the node already exists in _openSet and calls updateParent if so
    * @param openNode : The node to expand
    */
   void expandNode(SearchNode& openNode);

   /**
    * calculates the Heuristic of the node with the distance formula
    * @param node : node to calculate for
    * @return : the heuristic
    */
   static inline double calculateHeuristic(const SearchNode& start, const SearchNode& dest){
       double xTerm = (dest.x_coord - start.x_coord) * (dest.x_coord - start.x_coord);
       double yTerm = (dest.y_coord - start.y_coord) * (dest.y_coord - start.y_coord);
       return sqrt(xTerm + yTerm);
   }
};
