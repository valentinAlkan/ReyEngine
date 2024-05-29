#pragma once
#include "Application.h"

//class to hold node info for a 2d map
class SearchNode {
public:
   //operators
   inline bool operator==(const SearchNode other){
      return id == other.id;
   }
   inline bool operator!=(const SearchNode other){
      return id != other.id;
   }
   inline bool operator>(const SearchNode other){
      return combinedCost > other.combinedCost;
   }
   inline bool operator<(const SearchNode other){
      return combinedCost < other.combinedCost;
   }

   unsigned long long id = Application::generateUniqueValue();
   int xCoord, yCoord;
   float heuristic, cost, combinedCost;
   SearchNode *parent;

   SearchNode() : parent(nullptr){}
   SearchNode(int x, int y, float _heuristic, float _cost, SearchNode *_parent = nullptr) : xCoord(x), yCoord(y), heuristic(_heuristic) ,cost(_cost), combinedCost(heuristic + cost),parent(_parent){}

   /**
    * updates the parent node if the cost is less than the cost from the previous parent
    * @param _parent : the parent to update too
    * @param _cost : the cost to get to the node from the new parent
    */
   void updateParent(SearchNode *_parent, float _cost);
};
