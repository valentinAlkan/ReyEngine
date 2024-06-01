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
   float heuristic, baseCost, cost, combinedCost;
   int x_coord, y_coord;
   SearchNode *parent;
   std::map<SearchNode*, float> connections;

   SearchNode() : parent(nullptr){}
   SearchNode(float _heuristic, float _cost, SearchNode *_parent = nullptr) : heuristic(_heuristic) ,cost(_cost), combinedCost(heuristic + cost),parent(_parent){}

   /**
    * updates the parent node if the cost is less than the cost from the previous parent
    * @param _parent : the parent to update too
    * @param _cost : the cost to get to the node from the new parent
    */
   void updateParent(SearchNode *_parent, float _cost);

   /**
    * calculates the cost to get to this node from the start node
    * @param _cost : cost of the path to get to the parent node
    * @param connectionCost : the cost modifer to enter the tile from this connection
    */
   void setCost(float _cost, float connectionCost = 1);

   /**
    * sets the Heuristic value
    * @param _heuristic : the heuristic
    */
   void setHeuristic(float _heuristic);

   /**
    * adds the heuristic to the cost and saves in combinedCost
    */
   void calculateCombinedCost();

   /**
    * inserts the connection into the connections map
    * @param cost : the cost modifier to connect to the connection
    * @param connection : the connection
    */
   void addConnection(float cost, SearchNode *connection);

   /**
    * sets the x_coord and the y_coord of the node
    * @param x_coord : the desired x_coord
    * @param y_coord : the desired y_coord
    */
   void setCoords(int x_coord, int y_coord);
};
