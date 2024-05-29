#pragma once

#include "SearchNode.h"
#include <vector>
#include <queue>

//struct for comparing search nodes for the priority queue
struct Comparator{
   inline bool operator()(const SearchNode a, const SearchNode b){
      return a.combinedDistance > b.combinedDistance;
   }
};

//Class to perform AStar search
class AStar {
public:
   std::priority_queue<SearchNode, vector<SearchNode>,Comparator> expandSet;

   /**
    * places all connections in the priority queue and overwrites parents if the combined distance is less
    */
   void expandNode(SearchNode node);

   /**
    * gets the heuristic for that node to the goal
    * @param node : node to calculate heuristic for
    * @return : the heuristic
    */
    float getHeuristic(SearchNode node);

private:
   int _xStart, _yStart;
   int _xGoal, _yGoal;
};
