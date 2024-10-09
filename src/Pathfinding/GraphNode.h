#pragma once
#include <set>
#include "RefWrapCompare.h"
#include "DrawInterface.h"

namespace ReyEngine {
   /// Holds information needed to conduct a search, such as weights and connections;
   // All raw-pointers are NON-OWNING
   class GraphNode {
      static constexpr double WEIGHT_BIAS = 10;
   public:
      struct Connection {
         Connection(GraphNode* a, GraphNode* b): a(a), b(b){}
         Connection(const Connection&) = delete;
         GraphNode* a;
         GraphNode* b;
      };
      GraphNode(const Vec2<int>& coord): _coord(coord){}
      void addConnection(GraphNode& other, bool twoWay=true){
         auto ptr = std::make_unique<Connection>(this, &other);
         _connections.emplace(std::move(ptr));
      }
      GraphNode(const GraphNode&) = delete;
      using FCost = double;
      using GCost = double;
      using HCost = double;

      inline std::optional<std::reference_wrapper<GraphNode>> getParent(){
         if (_parent) return *_parent;
         return {};
      }
      GraphNode* _parent = nullptr;
      inline Vec2<int> getCoords() const {return _coord;}
      const std::set<std::unique_ptr<Connection>>& getConnections() const {return _connections;}
      bool operator==(const GraphNode& other) const {return _coord == other._coord;}
      inline void setWeight(double weight){_weight = weight + WEIGHT_BIAS;}
   protected:
      FCost getFCost(){return _fcost;}
      HCost getHCost(){return _hcost;}
      GCost getGCost(){return _gcost;}
      std::set<std::unique_ptr<Connection>> _connections;
      void setHCost();
      void setGCost();
      GCost _gcost; //the cost to get back to the start
      HCost _hcost; //heuristic cost
      FCost _fcost; //gcost + hcost
      const Vec2<int> _coord;
      double _weight = WEIGHT_BIAS;
   private:
      friend class AStar;
   };
}