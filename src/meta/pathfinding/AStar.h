#pragma once

#include "SearchNode.h"
#include <vector>
#include <queue>


//Class to perform AStar search
class AStar {
private:
   std::vector<SearchNode> _openSet;
   std::vector<SearchNode> _closeSet;
   std::priority_queue<SearchNode, std::vector<SearchNode>, greater<SearchNode>> _searchSet;

};
