#pragma once
#include "GraphNode.h"

//class to preform an AStar algorithm
namespace ReyEngine {
   class AStar {
   public:
      AStar(unsigned int sizeX, unsigned int sizeY): _sizeX(sizeX), _sizeY(sizeY){
         for (int x=0; x<sizeX; x++){
            for (int y=0; y<sizeX; y++){
               const auto& coord = Vec2<int>(x,y);
               auto node = std::make_unique<GraphNode>(coord);
               _graph.emplace(std::pair<unsigned int, Vec2<int>>(getIndex(coord), coord));
            }
         }
      }
      inline void addConnection(const Vec2<int>& src, const Vec2<int>& dst){
         auto found = _graph.find(getIndex(src));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode src @ " + src.toString() + " Not found");
         auto& srcNode = found->second;

         found = _graph.find(getIndex(dst));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode dst @ " + dst.toString() + " Not found");
         auto& dstNode = found->second;
         srcNode.addConnection(dstNode);
      }
      constexpr unsigned int getIndex(const Vec2<int>& coord){
         return coord.y * _sizeX + coord.x;
      }
      GraphNode& getGraphNode(const Vec2<int>& coords){
         return _graph.at(getIndex(coords));
      };
      //returns goal if path is valid
      std::optional<std::reference_wrapper<SearchNode>> findPath(GraphNode& graphStart, GraphNode& graphGoal);
   private:
      const unsigned int _sizeX;
      const unsigned int _sizeY;
      /// The map that AStar considers as a space to search.
      std::map<unsigned int, GraphNode> _graph;
      /// The frontier represents the fog of war - that is to say, it should always return the most promising connected node that
      /// has yet to be visited
      std::priority_queue<std::reference_wrapper<SearchNode>, std::vector<std::reference_wrapper<SearchNode>>, SearchNode::CombinedCostComparator> _frontier;
      /// The set of nodes that have already been visited and searched for connections
      std::set<std::reference_wrapper<GraphNode>, RefWrapperCompare> _visitedNodes;
      void queryGraph(GraphNode& openNode, const GraphNode& goal);
   };
}