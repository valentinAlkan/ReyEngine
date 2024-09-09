#pragma once
#include "Application.h"
#include <set>

//class to hold node info for a 2d map
class SearchNode : std::enable_shared_from_this<SearchNode> {
public:
   //operators
   inline bool operator==(const SearchNode& other) const {
      return id == other.id;
   }
   inline bool operator!=(const SearchNode& other) const {
      return id != other.id;
   }
   inline bool operator>(const SearchNode& other) const {
      return combinedCost > other.combinedCost;
   }
   inline bool operator<(const SearchNode& other) const {
      return combinedCost < other.combinedCost;
   }

   unsigned long long id = ReyEngine::Application::generateUniqueValue();
   float cost = 0;
   float heuristic, baseCost, combinedCost;
   int x_coord, y_coord;
   bool isStart = false;
   std::shared_ptr<SearchNode> parent;
   std::map<std::shared_ptr<SearchNode>, float> connections;
   std::set<std::shared_ptr<SearchNode>> references; //nodes that connect TO this node - so we can let them know we've been deleted.

   SearchNode(int x, int y): x_coord(x), y_coord(y){}
   ~SearchNode();
   /**
    * updates the parent node if the cost is less than the cost from the previous parent
    * @param _parent : the parent to run too
    * @param _cost : the cost to get to the node from the new parent
    * @param connectionCost : the modifier to connect to this node
    */
   void updateParent(std::shared_ptr<SearchNode> _parent, float _cost, float connectionCost);

   /**
    * calculates the cost to get to this node from the start node
    * @param _cost : cost of the path to get to the parent node
    * @param connectionCost : the cost modifier to enter the tile from this connection
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
   void removeConnection(std::shared_ptr<SearchNode> connection);
};
