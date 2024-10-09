#pragma once
#include "GraphNode.h"
#include <queue>

//class to preform an AStar algorithm
namespace ReyEngine {
   class AStar {
   public:
      AStar(const AStar&) = delete;
      AStar(unsigned int sizeX, unsigned int sizeY);
      inline void addConnection(const Vec2<int>& src, const Vec2<int>& dst, double weight){
         auto found = _graph.find(getIndex(src));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode src @ " + src.toString() + " Not found");
         auto& srcNode = found->second;

         found = _graph.find(getIndex(dst));
         if (found == _graph.end()) throw std::runtime_error("AStar GraphNode dst @ " + dst.toString() + " Not found");
         auto& dstNode = found->second;
         srcNode->addConnection(*dstNode, true);
      }
      inline constexpr unsigned int getIndex(const Vec2<int>& coord) const {return coord.y * _sizeX + coord.x;}
      inline GraphNode& operator[](const Vec2<int>& coords){
         return *(_graph.at(getIndex(coords)).get());
      };

      inline std::optional<std::reference_wrapper<GraphNode>> at (const Vec2<int>& coords){
         if (coords.x < _sizeX && coords.y < _sizeY){
            auto index = getIndex(coords);
            auto& ptr = _graph[index];
            return *ptr;
         }
         return {};
      };

      std::vector<std::reference_wrapper<GraphNode>> getNeighbors(const Vec2<int>& pos, bool includeCorners=true);
      /// Find a valid astar path
      /// \param graphStart
      /// \param graphGoal
      /// \return
      std::optional<std::reference_wrapper<GraphNode>> findPath(GraphNode& graphStart, GraphNode& graphGoal);
      /// Extract path from goal node
      /// \return path from start to goal
      std::vector<Vec2<int>> extractPath(const GraphNode& goal);
      /// Combines find and extract into one operation
      /// \param graphStart : the start node
      /// \param graphGoal : the end node
      /// \return a vector representing the path from start to goal
      inline std::vector<Vec2<int>> findAndExtractPath(GraphNode& graphStart, GraphNode& graphGoal) {
         auto found = findPath(graphStart, graphGoal);
         if (found) return extractPath(found.value());
         return {};
      };

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
         node._gcost = node._weight+ (parent ? parent->getGCost() : 0);
         node._fcost = node._hcost + node._gcost;
         node._parent = parent;
      }
      static inline double getHeuristic(const GraphNode& start, const GraphNode& dest){
         double _x = dest._coord.x - start._coord.x;
         double _y =  dest._coord.y - start._coord.y;
         return sqrt(_x * _x + _y * _y);
      }
   };
}