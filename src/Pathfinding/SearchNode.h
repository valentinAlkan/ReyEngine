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

   unsigned long long id = ReyEngine::Application::generateUniqueValue();
   float cost = 0;
   float heuristic, baseCost, combinedCost;
   int x_coord, y_coord;
   bool isStart = false;
   std::shared_ptr<SearchNode> parent;
   std::map<std::shared_ptr<SearchNode>, float> connections;

   SearchNode() : parent(nullptr){}

   /**
    * updates the parent node if the cost is less than the cost from the previous parent
    * @param _parent : the parent to update too
    * @param _cost : the cost to get to the node from the new parent
    * @param connectionCost : the modifier to connect to this node
    */
   void updateParent(std::shared_ptr<SearchNode> _parent, float _cost, float connectionCost);

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
   void addConnection(float cost, std::shared_ptr<SearchNode> connection);

   /**
    * sets the x_coord and the y_coord of the node
    * @param x_coord : the desired x_coord
    * @param y_coord : the desired y_coord
    */
   void setCoords(int xCoord, int yCoord);
};
