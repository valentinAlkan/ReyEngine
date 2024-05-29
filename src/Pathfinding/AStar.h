#pragma once
#include "SearchNode.h"

//class to preform an AStar algorithm
struct coord{
   int x, y;
};
class AStar {
public:
   AStar(coord start, coord goal);
   AStar(coord start, coord goal, SearchNode[][] *map);


};
