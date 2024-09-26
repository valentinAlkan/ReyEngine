#pragma once
#include "Application.h"
#include "StrongUnits.h"
#include "RefWrapCompare.h"
#include <set>

namespace ReyEngine{
   //Holds information about the ongoing search for a path. Not used to hold weights or connections. Does hold combined weights.
   class SearchNode {
   private:
      const Vec2<int> _coord;
      double _heuristic = 0;
      double _pathCost = 0;
      std::optional<std::reference_wrapper<SearchNode>> _parent;
      std::set<std::reference_wrapper<SearchNode>, RefWrapperCompare> _references; //nodes that connect TO this node - so we can let them know we've been deleted.
   public:
      SearchNode(const Vec2<int>& coord): _coord(coord){}
      inline void setParent(std::optional<std::reference_wrapper<SearchNode>> parent){_parent = parent;}
      inline void addCost(double _cost){ _pathCost = _cost + (_parent ? _parent->get()._pathCost : 0);}
      inline void setHeuristic(double heuristic){ _heuristic = heuristic;}
      inline Vec2<int> getCoords() const {return _coord;}
      inline std::optional<std::reference_wrapper<SearchNode>> getParent() const {return _parent;}
      void reset(){_pathCost = 0;}
   protected:
      struct CombinedCostComparator {
         bool operator()(const std::reference_wrapper<SearchNode> &a, const std::reference_wrapper<SearchNode> &b) const {
            return a.get()._pathCost > b.get()._pathCost;
         }
      };
      friend class AStar;
   };
}