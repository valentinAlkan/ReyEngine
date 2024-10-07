#pragma once
#include <set>
#include "RefWrapCompare.h"
#include "DrawInterface.h"

namespace ReyEngine {
   /// Holds information needed to conduct a search, such as weights and connections;
   // All raw-pointers are NON-OWNING and NON-NULL
   class GraphNode {
   public:
      struct Connection {
         Connection(GraphNode* a, GraphNode* b, double weight): a(a), b(b), weight(weight){}
         Connection(const Connection&) = delete;
         GraphNode* a;
         GraphNode* b;
         double weight;
      };
      using FCost = double;
      using GCost = double;
      using HCost = double;

      const Vec2<int> _coord;
      GCost _gcost; //the cost to get back to the start
      HCost _hcost; //heuristic cost
      FCost _fcost; //gcost + hcost
      FCost getFCost(){return _fcost;}
      HCost getHCost(){return _hcost;}
      GCost getGCost(){return _gcost;}
      inline std::optional<std::reference_wrapper<GraphNode>> getParent(){
         if (_parent){
            return *_parent;
         }
         return {};
      }
      GraphNode* _parent = nullptr;
      GraphNode(const Vec2<int>& coord): _coord(coord){}
      void addConnection(GraphNode& other, double weight, bool twoWay=true){
         auto ptr = std::make_unique<Connection>(this, &other, weight);
         _connections.emplace(std::move(ptr));
      }
      GraphNode(const GraphNode&) = delete;
      inline Vec2<int> getCoords() const {return _coord;}
      const std::set<std::unique_ptr<Connection>>& getConnections() const {return _connections;}
      bool operator==(const GraphNode& other) const {return _coord == other._coord;}
   protected:
      std::set<std::unique_ptr<Connection>> _connections;
      void setHCost();
      void setGCost();
   private:
      friend class SearchNode;
      friend class AStar;
   };
}