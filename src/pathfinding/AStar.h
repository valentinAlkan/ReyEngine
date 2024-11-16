#pragma once
#include <queue>
#include <set>
#include "DrawInterface.h"
#include "MathUtils.h"

//class to preform an AStar algorithm
namespace ReyEngine {
   namespace PathFinding{
      class AStar2D {
      public:
         class GraphNode {
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
            inline void setWeight(double weight){_weight = weight;}
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
            double _weight = 1.0;
         private:
            friend class AStar2D;
         };

         AStar2D(const AStar2D&) = delete;
         AStar2D(unsigned int sizeX, unsigned int sizeY);
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
         static std::vector<Vec2<int>> extractPath(const GraphNode& goal);
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
            return ReyEngine::Math::fsqrt(_x * _x + _y * _y);
         }
      };
   }
}