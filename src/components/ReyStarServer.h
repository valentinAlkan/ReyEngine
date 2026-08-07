#pragma once
#include <thread>
#include <map>
#include <stack>
#include <memory>
#include <concepts>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include "ReyEngine.h"

namespace ReyStar {
   // Any tile type usable with AStar must expose:
   //  - coord: its position
   //  - weight: nav cost of entering this tile (<= 0 means impassable)
   //  - neighborCoords(): adjacent coordinates - grid type and wrapping agnostic
   //  - distanceTo(other): heuristic
   template <typename T>
   concept ReyStarTileable = requires(const T& t) {
      { t.coord } -> std::convertible_to<ReyEngine::Pos<int>>;
      { t.weight } -> std::convertible_to<float>;
      { t.neighborCoords() } -> std::convertible_to<std::vector<std::pair<int, int>>>;
      { t.distanceTo(t) } -> std::convertible_to<float>;
   };

   struct ReyStarTile {
      ReyEngine::Pos<int> coord;
      float weight;
      virtual std::vector<std::pair<int, int>> neighborCoords() = 0;
      virtual float distanceTo(const ReyStarTile&) = 0;
   };

   using RequestId = unsigned long long;

   template <ReyStarTileable T>
   using TileMap = std::map<std::pair<int, int>, T>;

   template <ReyStarTileable T>
   struct PathTile{
      PathTile() = default;
      PathTile(const T* thisTile, std::unique_ptr<PathTile<T>>&& nextTile)
      : thisTile(thisTile)
      , nextTile(std::move(nextTile))
      {}
      PathTile(PathTile&& other){
         *this = std::move(other);
      }
      PathTile& operator=(PathTile&& other) noexcept {
         thisTile = other.thisTile;
         nextTile = std::move(other.nextTile);
         pathCost = other.pathCost;
         return *this;
      }
      const T* thisTile{};
      std::unique_ptr<PathTile<T>> nextTile; //stays null for invalid paths
      float pathCost = 0;
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   template <ReyStarTileable T>
   PathTile<T> aStarAlgorithm(const TileMap<T>& tileMap, const T& start, const T& goal) {
      // Node structure for the priority queue
      struct Node {
         const T* tile;
         float gScore;  // Cost from start
         float fScore;  // gScore + heuristic

         bool operator>(const Node& other) const {
            return fScore > other.fScore;  // Min heap
         }
      };

      // Hash function for tile coordinates
      auto coordHash = [](const std::pair<int, int>& coord) {
         return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 1);
      };

      std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
      std::unordered_set<std::pair<int, int>, decltype(coordHash)> closedSet(0, coordHash);
      std::unordered_map<std::pair<int, int>, float, decltype(coordHash)> gScores(0, coordHash);
      std::unordered_map<std::pair<int, int>, const T*, decltype(coordHash)> cameFrom(0, coordHash);

      // Initialize with start node
      gScores[{start.coord.x, start.coord.y}] = 0.0f;
      openSet.push({&start, 0.0f, start.distanceTo(goal)});

      while (!openSet.empty()) {
         // Get the node with lowest f-score
         Node current = openSet.top();
         openSet.pop();

         auto currentCoord = std::make_pair(current.tile->coord.x, current.tile->coord.y);

         // Skip if already visited
         if (closedSet.count(currentCoord)) {
            continue;
         }

         // Mark as visited
         closedSet.insert(currentCoord);

         // Check if we reached the goal
         if (current.tile == &goal) {
            // Reconstruct path by walking backwards from goal to start
            std::vector<const T*> path;
            const T* currentTile = &goal;

            //calculate pathCost
            float cost = currentTile->weight;

            // Build path from goal back to start
            path.push_back(currentTile);
            while (currentTile != &start) {
               auto parentIt = cameFrom.find({currentTile->coord.x, currentTile->coord.y});
               if (parentIt == cameFrom.end()) {
                  break;  // Shouldn't happen, but safety check
               }
               currentTile = parentIt->second;
               cost += currentTile->weight;
               path.push_back(currentTile);
            }

            // Reverse to get start-to-goal order
            std::reverse(path.begin(), path.end());

            // Build linked list from the end backwards
            std::unique_ptr<PathTile<T>> nextNode;
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
               nextNode = std::make_unique<PathTile<T>>(*it, std::move(nextNode));
            }

            // Return the head (which is now in nextNode), or empty if path is empty
            if (nextNode) {
               nextNode->pathCost = cost;
               return std::move(*nextNode);
            }
            // Start is goal
            return PathTile<T>{&start, nullptr};
         }

         // Explore neighbors - adjacency is entirely up to the tile type
         for (const auto& [nx, ny] : current.tile->neighborCoords()) {
            auto it = tileMap.find({nx, ny});
            if (it == tileMap.end()) {
               continue; // no tile at that coordinate
            }
            const T* neighbor = &it->second;

            // Skip impassable tiles
            if (neighbor->weight <= 0.0f) {
               continue;
            }

            auto neighborCoord = std::make_pair(nx, ny);

            // Skip if already visited
            if (closedSet.count(neighborCoord)) {
               continue;
            }

            // Calculate tentative g-score
            float tentativeG = current.gScore + neighbor->weight;

            // Check if this is a better path
            auto gIt = gScores.find(neighborCoord);
            if (gIt == gScores.end() || tentativeG < gIt->second) {
               // Update best path to this neighbor
               cameFrom[neighborCoord] = current.tile;
               gScores[neighborCoord] = tentativeG;

               float fScore = tentativeG + neighbor->distanceTo(goal);
               openSet.push({neighbor, tentativeG, fScore});
            }
         }
      }

      // No path found
      return PathTile<T>{nullptr, nullptr};
   }

   /////////////////////////////////////////////////////////////////////////////////////////
   template <ReyStarTileable T>
   struct PathResult{
      PathResult() = default;
      PathResult(PathResult&& other){
         *this = std::move(other);
      }
      PathResult& operator=(PathResult&& other) noexcept {
         path = std::move(other.path);
         pathCost = other.pathCost;
         requestId = other.requestId;
         processTime = other.processTime;
         pathFound = other.pathFound;
         start = other.start;
         goal = other.goal;
         return *this;
      }
      RequestId requestId;
      std::chrono::microseconds processTime;
      bool pathFound;
      PathTile<T> path;
      float pathCost;
      T* start;
      T* goal;
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   template <ReyStarTileable T>
   class ReyStarServer {
   public:
      ReyStarServer(TileMap<T>& tiles)
      : tiles(tiles)
      {
         //generate an appropriate number of workers
         unsigned int num_cores = std::thread::hardware_concurrency();
         _workers.reserve(num_cores);
         for (auto i = 0u; i < num_cores; i++){
            _workers.emplace_back(i, *this);
         }
      }
      ReyStarServer(const ReyStarServer&) = delete;
      ReyStarServer(const ReyStarServer&&) = delete;
      ReyStarServer& operator=(const ReyStarServer&) = delete;
      ReyStarServer& operator=(const ReyStarServer&&) = delete;

      std::future<PathResult<T>> requestPath(T& start, T& goal) {
         return requestPathAndReturnId(start, goal).second;
      }

      std::pair<RequestId, std::future<PathResult<T>>> requestPathAndReturnId(T& start, T& goal) {
         //test to see if they are the same, if they are just short circuit
         auto id = _nextRequestId++;
         if (start.coord == goal.coord){
            PathResult<T> path;
            path.pathFound = true;
            path.path.thisTile = &start;
            path.processTime = std::chrono::microseconds(0);
            path.requestId = id;
            path.pathCost = 0;
            auto promise = std::promise<PathResult<T>>();
            promise.set_value(std::move(path));
            return {id, promise.get_future()};
         }

         auto res = _openOrders.try_emplace(id);

         //assume success
         auto& newOrder = res.first->second.first;
         newOrder.requestId = id;
         newOrder.requestTime = std::chrono::steady_clock::now();
         newOrder.start = &start;
         newOrder.goal = &goal;
         newOrder.heuristic = start.distanceTo(goal);
         newOrder.promise = &res.first->second.second;

         auto getWorkLoad = [](WorkOrder& order) -> WorkLoad {
            if (order.heuristic >= MEDIUM_THRESHOLD) return WorkLoad::HEAVY;
            if (order.heuristic >= LIGHT_THRESHOLD) return WorkLoad::MEDIUM;
            return WorkLoad::LIGHT;
         };

         //assign the job to an appropriate worker given the expected workload of the request
         _queueMtx.lock();
         switch (getWorkLoad(newOrder)){
            case WorkLoad::LIGHT: _lightQueue.push(&newOrder); break;
            case WorkLoad::MEDIUM: _mediumQueue.push(&newOrder); break;
            case WorkLoad::HEAVY: _heavyQueue.push(&newOrder); break;
         }
         _requestCount++; //make sure this is always protected by the mutex
         _queueMtx.unlock();
         _condition_variable.notify_one();
         return {newOrder.requestId, newOrder.promise->get_future()};
      }

      void start() {} //workers launch themselves in the constructor; kept for API compatibility

   protected:
      static constexpr float LIGHT_THRESHOLD = 10;
      static constexpr float MEDIUM_THRESHOLD = 30;

      enum class WorkLoad {LIGHT, MEDIUM, HEAVY};

      struct WorkOrder{
         RequestId requestId;
         std::chrono::steady_clock::time_point requestTime;
         T* start;
         T* goal;
         PathResult<T> result;
         float heuristic;
         std::promise<PathResult<T>>* promise;
      };

      struct Worker{
         Worker(int workerId, ReyStarServer& server)
         : workerId(workerId)
         , _server(server)
         {
            _t = std::thread(&Worker::work, this);
         }
         Worker(const Worker&) = delete;
         Worker(Worker&& other)
         : workerId(other.workerId)
         , _server(other._server)
         , _t(std::move(other._t))
         {
         }
         Worker& operator=(const Worker&) = delete;
         Worker& operator=(const Worker&&) = delete;
         ~Worker(){
            _cancel = true;
            _shutdown = true;
            if (_t.joinable()) _t.join();
         }
         const int workerId;
         std::atomic<bool> _cancel = false;
      private:
         void work(){
            while (!_shutdown){
               WorkOrder* order;
               _cancel = false;
               std::unique_lock<std::mutex> lock(_server._queueMtx);
               _server._condition_variable.wait(lock, [&] {
                  // Wait for data. Predicate is protected by mutex during eval.
                  if (_server._requestCount) {
                     order = _server.getOpenWorkOrder();
                     return true;
                  }
                  return false;
               });
               //order is now valid.
               lock.unlock();  // Release lock before processing

               bool done = false;
               auto& promise = order->promise;
               if (!_cancel) {
                  auto solution = aStarAlgorithm<T>(_server.tiles, *order->start, *order->goal);

                  // Build the result
                  PathResult<T> result;
                  result.requestId = order->requestId;
                  result.pathFound = (solution.thisTile != nullptr);  // Check before moving
                  result.path = std::move(solution);
                  result.processTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - order->requestTime);
                  result.start = order->start;
                  result.goal = order->goal;
                  result.pathCost = solution.pathCost;
                  promise->set_value(std::move(result));
                  done = true;
               }
               if (!done){
                  promise->set_exception(nullptr);
               }
            }
         }
         ReyStarServer& _server;
         std::thread _t;
         bool _shutdown = false;
      };

      WorkOrder* getOpenWorkOrder(){ //favors light then medium then heavy
         WorkOrder* order = nullptr;
         if (!_lightQueue.empty()) {
            order = _lightQueue.top();
            _lightQueue.pop();
         } else if (!_mediumQueue.empty()) {
            order = _mediumQueue.top();
            _mediumQueue.pop();
         } else if (!_heavyQueue.empty()) {
            order = _heavyQueue.top();
            _heavyQueue.pop();
         }
         _requestCount--;
         return order;
      }

      std::map<RequestId, std::pair<WorkOrder, std::promise<PathResult<T>>>> _openOrders;
      std::vector<Worker> _workers;

      std::mutex _queueMtx; //mutex for these queues
      std::condition_variable _condition_variable;

      using WorkOrderStack = std::stack<WorkOrder*, std::vector<WorkOrder*>>;
      WorkOrderStack _lightQueue; //light-ish requests
      WorkOrderStack _mediumQueue; //medium-ish requests
      WorkOrderStack _heavyQueue; //all others

      unsigned int _requestCount = 0; //must be protected by mutex

      RequestId _nextRequestId = 0;

      TileMap<T>& tiles;
      friend class Worker;
   };
}
