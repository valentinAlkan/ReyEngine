#pragma once
#include "Application.h"
#include <set>

//class to hold node info for a 2d map
class SearchNode {
public:
    using NodeId = unsigned long long;
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
    struct RefWrapperCompare {
        template<typename T>
        bool operator()(const std::reference_wrapper<T>& lhs, const std::reference_wrapper<T>& rhs) const {
            return std::less<T>{}(lhs.get(), rhs.get());
        }
    };

   NodeId id = ReyEngine::Application::generateUniqueValue();
   double cost = 0;
   double heuristic, baseCost, combinedCost;
   int x_coord, y_coord;
   std::optional<std::reference_wrapper<SearchNode>> parent;
   std::map<std::reference_wrapper<SearchNode>, double, RefWrapperCompare> connections;
   std::set<std::reference_wrapper<SearchNode>, RefWrapperCompare> references; //nodes that connect TO this node - so we can let them know we've been deleted.

   SearchNode(int x, int y): x_coord(x), y_coord(y){}
   ~SearchNode();
   /**
    * updates the parent node if the cost is less than the cost from the previous parent
    * @param _parent : the parent to run too
    * @param _cost : the cost to get to the node from the new parent
    * @param connectionCost : the modifier to connect to this node
    */
   void updateParent(SearchNode& _parent, double _cost, double connectionCost);

   /**
    * calculates the cost to get to this node from the start node
    * @param _cost : cost of the path to get to the parent node
    * @param connectionCost : the cost modifier to enter the tile from this connection
    */
   void setCost(double _cost, double connectionCost = 1);

   /**
    * sets the Heuristic value
    * @param _heuristic : the heuristic
    */
   void setHeuristic(double _heuristic);

   /**
    * adds the heuristic to the cost and saves in combinedCost
    */
   void calculateCombinedCost();

   /**
    * inserts the connection into the connections map
    * @param cost : the cost modifier to connect to the connection
    * @param connection : the connection
    */
   void addConnection(std::reference_wrapper<SearchNode> connection, double cost);
   void removeConnection(std::reference_wrapper<SearchNode> connection);
};
