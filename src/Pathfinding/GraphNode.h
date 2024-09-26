#pragma once
#include "RefWrapCompare.h"
#include "DrawInterface.h"
#include "SearchNode.h"

namespace ReyEngine {
   /// Holds information needed to conduct a search, such as weights and connections;
   class SearchNode;
   class GraphNode {
   public:
      using Cost = double;
      const Vec2<int> _coord;
      Cost _cost;
      GraphNode(const Vec2<int> coord): _coord(coord), _searchNode(coord){}
      inline double getHeuristic(const GraphNode& dest){
         double x = (dest._coord.x - _coord.x) * (dest._coord.x - _coord.x);
         double y = (dest._coord.y - _coord.y) * (dest._coord.y - _coord.y);
         return sqrt(x + y);
      }
      void addConnection(const std::reference_wrapper<GraphNode>& node){
         _connections.emplace(std::pair<Cost, std::reference_wrapper<GraphNode>>(node.get()._cost, node));
      }
      inline Vec2<int> getCoords() const {return _coord;}
      SearchNode& getSearchNode(){return _searchNode;}
      void setCost(double cost){_cost = cost;}
      const std::map<Cost, std::reference_wrapper<GraphNode>> getConnections() const {return _connections;}
   protected:
      SearchNode _searchNode;
      std::map<Cost, std::reference_wrapper<GraphNode>> _connections;
   private:
      friend class SearchNode;
      friend class AStar;
   };
}