#pragma once
#include "GraphNode.h"
#include <queue>

//class to preform an AStar algorithm
namespace ReyEngine {
   class AStar {
   public:
      AStar(const AStar&) = delete;
      AStar(unsigned int sizeX, unsigned int sizeY): _sizeX(sizeX), _sizeY(sizeY){
         for (int x=0; x<sizeX; x++){
            for (int y=0; y<sizeY; y++){
               const auto& coord = Vec2<int>(x,y);
               auto node = std::make_unique<GraphNode>(coord);
               _graph[getIndex(coord)] = std::move(node);
            }
         }
      }

      inline void addConnection(const Vec2<int>& src, const Vec2<int>& dst, double weight){
         auto found = _graph.find(getIndex(src));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode src @ " + src.toString() + " Not found");
         auto& srcNode = found->second;

         found = _graph.find(getIndex(dst));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode dst @ " + dst.toString() + " Not found");
         auto& dstNode = found->second;
         srcNode->addConnection(*dstNode, weight, true);
      }
      constexpr unsigned int getIndex(const Vec2<int>& coord){
         return coord.y * _sizeX + coord.x;
      }
      GraphNode& operator[](const Vec2<int>& coords){
         return *(_graph.at(getIndex(coords)).get());
      };

      std::optional<std::reference_wrapper<GraphNode>> at(const Vec2<int>& coords){
         if (coords.x < _sizeX && coords.y < _sizeY){
            auto index = getIndex(coords);
            auto& ptr = _graph[index];
            return *ptr;
         }
         return {};
      };

      std::vector<std::reference_wrapper<GraphNode>> getNeighbors(const Vec2<int>& pos, bool includeCorners=true){
         std::vector<std::reference_wrapper<GraphNode>> retval;
         std::optional<std::reference_wrapper<GraphNode>> node;
         for (auto _x : {-1,0,1}){
            for (auto _y : {-1,0,1}){
               node = at({pos.x + _x, pos.y + _y});
               if (node && node.value().get()._coord != pos){
                  retval.push_back(node.value());
               }
            }
         }
         return retval;
      }
      //returns goal if path is valid
      std::optional<std::reference_wrapper<GraphNode>> findPath(GraphNode& graphStart, GraphNode& graphGoal);

      // Iterator class for AStar
      class Iterator {
      public:
         using iterator_category = std::forward_iterator_tag;
         using value_type = GraphNode;
         using pointer = GraphNode*;
         using reference = GraphNode&;
         Iterator(std::unordered_map<unsigned int, std::unique_ptr<GraphNode>>::iterator current): _current(current) {}
         reference operator*() const {return *(_current->second);}
         pointer operator->() const {return _current->second.get();}
         Iterator& operator++() {++_current;return *this;}
         Iterator operator++(int) {Iterator temp = *this;++(*this);return temp;}
         bool operator==(const Iterator& other) const {return _current == other._current;}
         bool operator!=(const Iterator& other) const {return !(*this == other);}
      private:
         std::unordered_map<unsigned int, std::unique_ptr<GraphNode>>::iterator _current;
      };

      Iterator begin() {return Iterator(_graph.begin());}
      Iterator end() {return Iterator(_graph.end());}

   private:
      const unsigned int _sizeX;
      const unsigned int _sizeY;
      /// The map that AStar considers as a space to search.
      std::unordered_map<unsigned int, std::unique_ptr<GraphNode>> _graph;
      /// The frontier represents the fog of war - that is to say, it should always return the most promising connected node that
      /// has yet to be visited

      /// The set of nodes that are eligible for searching. Parent and f_cost could be changed at any time.
      std::set<GraphNode*> _openSet;
      /// the set of nodes that have been searched already and will not be changed
      std::set<GraphNode*> _closedSet;
      ///May return nullptr if empty openset
      inline std::set<GraphNode*>::iterator getNodeLowestFCost(){
         if (_openSet.empty()) return _openSet.end();
         auto next = _openSet.begin();
         auto retval = next;
         for (/**/; next != _openSet.end(); ++next){
            if ((*next)->_fcost < (*retval)->_fcost){
               retval = next;
            }
         }
         return retval;
      };
      static inline void expandNode(GraphNode::Connection& connection, GraphNode* parent, const GraphNode& goal){
         auto& node = *(connection.b);
         node._hcost = getHeuristic(node, goal);
         node._gcost = connection.weight + (parent ? parent->getGCost() : 0);
         node._fcost = node._hcost + node._gcost;
         node._parent = parent;
      }
      static inline double getHeuristic(const GraphNode& start, const GraphNode& dest){
         double x = (dest._coord.x - start._coord.x) * (dest._coord.x - start._coord.x);
         double y = (dest._coord.y - start._coord.y) * (dest._coord.y - start._coord.y);
         return x + y;
      }
   };
}