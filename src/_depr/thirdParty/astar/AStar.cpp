#include "AStar.hpp"
#include <algorithm>
#include <math.h>

using namespace std::placeholders;

bool ReyEngine::AStar::Vec2i::operator == (const Vec2i& coordinates_)
{
   return (x == coordinates_.x && y == coordinates_.y);
}

ReyEngine::AStar::Node::Node(Vec2i coordinates_, Node *parent_)
{
   parent = parent_;
   coordinates = coordinates_;
   G = H = 0;
}

ReyEngine::AStar::uint ReyEngine::AStar::Node::getScore()
{
   return G + H;
}

ReyEngine::AStar::Generator::Generator()
{
   setDiagonalMovement(false);
   setHeuristic(&Heuristic::manhattan);
   direction = {
         { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },
         { -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }
   };
}

void ReyEngine::AStar::Generator::setWorldSize(Vec2i worldSize_)
{
   worldSize = worldSize_;
}

void ReyEngine::AStar::Generator::setDiagonalMovement(bool enable_)
{
   directions = (enable_ ? 8 : 4);
}

void ReyEngine::AStar::Generator::setHeuristic(HeuristicFunction heuristic_)
{
   heuristic = std::bind(heuristic_, _1, _2);
}

void ReyEngine::AStar::Generator::addCollision(Vec2i coordinates_)
{
   walls.push_back(coordinates_);
}

void ReyEngine::AStar::Generator::removeCollision(Vec2i coordinates_)
{
   auto it = std::find(walls.begin(), walls.end(), coordinates_);
   if (it != walls.end()) {
      walls.erase(it);
   }
}

void ReyEngine::AStar::Generator::clearCollisions()
{
   walls.clear();
}

ReyEngine::AStar::CoordinateList ReyEngine::AStar::Generator::findPath(Vec2i source_, Vec2i target_)
{
   Node *current = nullptr;
   NodeSet openSet, closedSet;
   openSet.reserve(100);
   closedSet.reserve(100);
   openSet.push_back(new Node(source_));

   while (!openSet.empty()) {
      auto current_it = openSet.begin();
      current = *current_it;

      for (auto it = openSet.begin(); it != openSet.end(); it++) {
         auto node = *it;
         if (node->getScore() <= current->getScore()) {
            current = node;
            current_it = it;
         }
      }

      if (current->coordinates == target_) {
         break;
      }

      closedSet.push_back(current);
      openSet.erase(current_it);

      for (uint i = 0; i < directions; ++i) {
         Vec2i newCoordinates(current->coordinates + direction[i]);
         if (detectCollision(newCoordinates) ||
             findNodeOnList(closedSet, newCoordinates)) {
            continue;
         }

         uint totalCost = current->G + ((i < 4) ? 10 : 14);

         Node *successor = findNodeOnList(openSet, newCoordinates);
         if (successor == nullptr) {
            successor = new Node(newCoordinates, current);
            successor->G = totalCost;
            successor->H = heuristic(successor->coordinates, target_);
            openSet.push_back(successor);
         }
         else if (totalCost < successor->G) {
            successor->parent = current;
            successor->G = totalCost;
         }
      }
   }

   CoordinateList path;
   while (current != nullptr) {
      path.push_back(current->coordinates);
      current = current->parent;
   }

   releaseNodes(openSet);
   releaseNodes(closedSet);

   return path;
}

ReyEngine::AStar::Node* ReyEngine::AStar::Generator::findNodeOnList(NodeSet& nodes_, Vec2i coordinates_)
{
   for (auto node : nodes_) {
      if (node->coordinates == coordinates_) {
         return node;
      }
   }
   return nullptr;
}

void ReyEngine::AStar::Generator::releaseNodes(NodeSet& nodes_)
{
   for (auto it = nodes_.begin(); it != nodes_.end();) {
      delete *it;
      it = nodes_.erase(it);
   }
}

bool ReyEngine::AStar::Generator::detectCollision(Vec2i coordinates_)
{
   if (coordinates_.x < 0 || coordinates_.x >= worldSize.x ||
       coordinates_.y < 0 || coordinates_.y >= worldSize.y ||
       std::find(walls.begin(), walls.end(), coordinates_) != walls.end()) {
      return true;
   }
   return false;
}

ReyEngine::AStar::Vec2i ReyEngine::AStar::Heuristic::getDelta(Vec2i source_, Vec2i target_)
{
   return{ abs(source_.x - target_.x),  abs(source_.y - target_.y) };
}

ReyEngine::AStar::uint ReyEngine::AStar::Heuristic::manhattan(Vec2i source_, Vec2i target_)
{
   auto delta = std::move(getDelta(source_, target_));
   return static_cast<uint>(10 * (delta.x + delta.y));
}

ReyEngine::AStar::uint ReyEngine::AStar::Heuristic::euclidean(Vec2i source_, Vec2i target_)
{
   auto delta = std::move(getDelta(source_, target_));
   return static_cast<uint>(10 * sqrt(pow(delta.x, 2) + pow(delta.y, 2)));
}

ReyEngine::AStar::uint ReyEngine::AStar::Heuristic::octagonal(Vec2i source_, Vec2i target_)
{
   auto delta = std::move(getDelta(source_, target_));
   return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
}
