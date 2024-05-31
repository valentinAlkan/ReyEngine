#pragma once
#include "SearchNode.h"

//class to preform an AStar algorithm
struct coord{
   int x, y;
};
class AStar {
public:
   AStar(coord start, coord goal);

private:
   //Map that is the search, first int is y, second is x
   std::map<int, std::map<int, SearchNode>> _searchMap;

   std::priority_queue<SearchNode, std::vector<SearchNode>, std::greater<SearchNode>> _expandedNodes;
   std::map<int, SearchNode> _closedSet;
   std::map<int, SearchNode> _openSet;
};
