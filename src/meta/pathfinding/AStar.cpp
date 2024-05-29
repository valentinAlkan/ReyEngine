#include "AStar.h"

float AStar::getHeuristic(SearchNode node) {
   float heuristic = sqrt((_xGoal - node.x_coord)*(_xGoal - node.x_coord) + (_yGoal - node.y_coord) * (_yGoal - node.y_coord));
   return heuristic;
}


