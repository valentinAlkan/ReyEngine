#pragma once
#include <math.h>
#include "Application.h"

using namespace std;
//class to hold node info to use in the pathfinding algorithm
class SearchNode {
public:
   int x_coord, y_coord;
   int id;
   SearchNode *parent;
   float combinedDistance;
   float heuristic;

   SearchNode() : parent(0){}
   SearchNode(int x, int y, SearchNode *_parent = 0) : x_coord(x), y_coord(y), parent(_parent), id(Application::generateUniqueValue()){}
};