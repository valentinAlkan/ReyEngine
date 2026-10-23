#pragma once
#include <thread>
#include <map>
#include <stack>
#include <memory>
#include <concepts>
#include <vector>
#include <functional>
#include <type_traits>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
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

   //premade template, or you can make your own
   template <typename Unused>
   struct ReyStarTileImpl {
      ReyEngine::Pos<int> coord;
      float weight;
      [[nodiscard]] std::vector<std::pair<int, int>> neighborCoords() const;
      [[nodiscard]] float distanceTo(const ReyStarTileImpl&) const;
   };

   using RequestId = unsigned long long;

   // Tiles can live in either of two containers. A TileMap holds any set of coordinates, holes and all; a Grid holds
   // a full rectangle, flat, which takes a fraction of the memory and finds a tile by arithmetic rather than a tree
   // search. The search, and the server, work with either (see findTile and tileKey).
   template <ReyStarTileable T>
   using TileMap = std::map<std::pair<int, int>, T>;

   /// A width x height rectangle of tiles, stored row by row in one block. Tile (x, y) has coord {x, y}; the grid
   /// sets it. Its size is fixed when it's made: tiles never move, so pointers to them stay good for the grid's life
   /// (the server's workers rely on it). Holes can't be left; make them impassable (weight 0) instead.
   template <ReyStarTileable T>
   class Grid {
   public:
      Grid() = default;
      Grid(int width, int height)
      : _width(std::max(0, width))
      , _height(std::max(0, height))
      , _tiles((size_t)_width * _height)
      {
         for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) _tiles[index(x, y)].coord = ReyEngine::Pos<int>(x, y);
         }
      }
      [[nodiscard]] int width() const {return _width;}
      [[nodiscard]] int height() const {return _height;}
      [[nodiscard]] size_t size() const {return _tiles.size();}
      [[nodiscard]] bool contains(int x, int y) const {return x >= 0 && y >= 0 && x < _width && y < _height;}
      /// where a tile is in the block (row by row). Only meaningful for coordinates in the grid.
      [[nodiscard]] size_t index(int x, int y) const {return (size_t)y * _width + x;}
      /// the tile at x, y, or null if that's outside the grid
      [[nodiscard]] T* find(int x, int y) {return contains(x, y) ? &_tiles[index(x, y)] : nullptr;}
      [[nodiscard]] const T* find(int x, int y) const {return contains(x, y) ? &_tiles[index(x, y)] : nullptr;}
      /// the tile at x, y. Throws std::out_of_range if that's outside the grid.
      [[nodiscard]] T& at(int x, int y) {return const_cast<T&>(std::as_const(*this).at(x, y));}
      [[nodiscard]] const T& at(int x, int y) const {
         if (!contains(x, y)) throw std::out_of_range("ReyStar::Grid: no tile at " + std::to_string(x) + "," + std::to_string(y));
         return _tiles[index(x, y)];
      }
      [[nodiscard]] T& operator[](size_t i) {return _tiles[i];}
      [[nodiscard]] const T& operator[](size_t i) const {return _tiles[i];}
      // every tile, row by row
      [[nodiscard]] auto begin() {return _tiles.begin();}
      [[nodiscard]] auto end() {return _tiles.end();}
      [[nodiscard]] auto begin() const {return _tiles.begin();}
      [[nodiscard]] auto end() const {return _tiles.end();}
   private:
      int _width = 0;
      int _height = 0;
      std::vector<T> _tiles;
   };

   /// the tile at x, y, or null if there isn't one
   template <ReyStarTileable T>
   const T* findTile(const TileMap<T>& tiles, int x, int y) {
      auto it = tiles.find({x, y});
      return it == tiles.end() ? nullptr : &it->second;
   }
   template <ReyStarTileable T>
   const T* findTile(const Grid<T>& tiles, int x, int y) {return tiles.find(x, y);}

   /// a number for a tile that no other tile in the container shares, for the search to file what it learns under. A
   /// grid uses the tile's place in the block; a map, both coordinates packed together.
   template <ReyStarTileable T>
   uint64_t tileKey(const TileMap<T>&, const T& tile) {
      return ((uint64_t)(uint32_t)tile.coord.x << 32) | (uint32_t)tile.coord.y;
   }
   template <ReyStarTileable T>
   uint64_t tileKey(const Grid<T>& tiles, const T& tile) {return tiles.index(tile.coord.x, tile.coord.y);}

   /// a container the search can find tiles of type T in
   template <typename Tiles, typename T>
   concept TileContainer = requires(const Tiles& tiles, const T& tile, int x) {
      { findTile(tiles, x, x) } -> std::convertible_to<const T*>;
      { tileKey(tiles, tile) } -> std::convertible_to<uint64_t>;
   };

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

      // Forward iterator that walks the linked list of tiles from start to goal.
      struct Iterator {
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const PathTile<T>;
         using pointer = const PathTile<T>*;
         using reference = const PathTile<T>&;

         explicit Iterator(const PathTile<T>* node) : _node(node) {}

         reference operator*() const { return *_node; }
         pointer operator->() const { return _node; }

         Iterator& operator++() {
            _node = _node->nextTile.get();
            return *this;
         }
         Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
         }

         bool operator==(const Iterator& other) const { return _node == other._node; }
         bool operator!=(const Iterator& other) const { return _node != other._node; }

      private:
         const PathTile<T>* _node;
      };

      [[nodiscard]] Iterator begin() const { return Iterator(thisTile ? this : nullptr); }
      [[nodiscard]] Iterator end() const { return Iterator(nullptr); }
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   template <ReyStarTileable T, typename Tiles> requires TileContainer<Tiles, T>
   PathTile<T> aStarAlgorithm(const Tiles& tiles, const T& start, const T& goal) {
      // Node structure for the priority queue
      struct Node {
         const T* tile;
         float gScore;  // Cost from start
         float fScore;  // gScore + heuristic

         bool operator>(const Node& other) const {
            return fScore > other.fScore;  // Min heap
         }
      };
      // what the search knows about a tile it has reached, filed under the tile's key (see tileKey, which never gives
      // two tiles the same one, so the table's hashing spreads them evenly)
      struct Visit {
         float gScore;                //cheapest cost found so far from the start
         const T* cameFrom = nullptr; //the tile before it on that cheapest way
         bool closed = false;         //expanded already: its cost is final
      };

      std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
      std::unordered_map<uint64_t, Visit> visits;

      // Initialize with start node
      visits[tileKey(tiles, start)] = {0.0f};
      openSet.push({&start, 0.0f, start.distanceTo(goal)});

      while (!openSet.empty()) {
         // Get the node with lowest f-score
         Node current = openSet.top();
         openSet.pop();

         // Skip if already visited (a cheaper way to it was queued, and taken, after this one)
         auto& visit = visits.at(tileKey(tiles, *current.tile));
         if (visit.closed) {
            continue;
         }

         // Mark as visited
         visit.closed = true;

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
               currentTile = visits.at(tileKey(tiles, *currentTile)).cameFrom;
               if (!currentTile) {
                  break;  // Shouldn't happen, but safety check
               }
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
            const T* neighbor = findTile(tiles, nx, ny);
            if (!neighbor) {
               continue; // no tile at that coordinate
            }

            // Skip impassable tiles
            float weight = neighbor->weight;
            if (weight <= 0.0f) {
               continue;
            }

            // Calculate tentative g-score
            float tentativeG = current.gScore + weight;

            // Check if this is a better path (one already expanded can't be: its cost is final)
            auto [it, isNew] = visits.try_emplace(tileKey(tiles, *neighbor), Visit{tentativeG, current.tile});
            if (!isNew) {
               if (it->second.closed || tentativeG >= it->second.gScore) {
                  continue;
               }
               it->second.gScore = tentativeG;
               it->second.cameFrom = current.tile;
            }
            float fScore = tentativeG + neighbor->distanceTo(goal);
            openSet.push({neighbor, tentativeG, fScore});
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
   /// Finds paths on worker threads (one per core) across a container of tiles: a TileMap (the default) or a Grid.
   /// The server keeps a reference to the container; its shape must not change while the server exists, but tiles'
   /// weights may (make them atomic if they change while searches run).
   ///
   /// It can be given a finisher: a function each result goes through on the worker, after the search and before the
   /// future is ready, to do any work on the path that shouldn't land on the caller's thread (straightening it, say).
   /// It may change the result, path included, and may run on any worker at once, so it must be safe to call from
   /// several threads. A request whose start is its goal is answered straight away, so its result goes through the
   /// finisher on the caller's thread.
   template <ReyStarTileable T, typename Tiles = TileMap<T>> requires TileContainer<Tiles, T>
   class ReyStarServer {
   public:
      using Finisher = std::function<void(PathResult<T>&)>;
      ~ReyStarServer()
      {
         //This cleanup code exists to solve the static destruction order problem
         // when the object is created with a static lifetime. Since the underlying threads might
         // not be woken, stopped, and deleted in time, we have to explicitly wake them and enforce that
         // they be destroyed.

         // When declared on the stack, the workers will be shut down cleanly and this code is redundant.
         {
            //set the flags under the queue mutex: a worker checks them (in its wait predicate) under the same mutex,
            // so it either sees them or is already waiting and gets the notification. Without the lock a worker
            // could check, miss the flags, then miss the wakeup and never return from wait.
            std::unique_lock<std::mutex> lock(_queueMtx);
            for (auto& worker : _workers) {
               worker.stop();
            }
         }
         _condition_variable.notify_all();
         //any orders still queued are destroyed with the queues, so their futures get a broken_promise error
         for (auto& worker : _workers) {
            worker.wait();
         }
      }
      explicit ReyStarServer(Tiles& tiles, Finisher finish = {})
      : tiles(tiles)
      , _finish(std::move(finish))
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
            path.start = &start;
            path.goal = &goal;
            if (_finish) _finish(path);
            auto promise = std::promise<PathResult<T>>();
            promise.set_value(std::move(path));
            return {id, promise.get_future()};
         }

         //the order owns its promise and is handed from the queue to a worker, which frees it once it's answered
         auto newOrder = std::make_unique<WorkOrder>();
         newOrder->requestId = id;
         newOrder->requestTime = std::chrono::steady_clock::now();
         newOrder->start = &start;
         newOrder->goal = &goal;
         newOrder->heuristic = start.distanceTo(goal);
         auto future = newOrder->promise.get_future();

         auto getWorkLoad = [](WorkOrder& order) -> WorkLoad {
            if (order.heuristic >= MEDIUM_THRESHOLD) return WorkLoad::HEAVY;
            if (order.heuristic >= LIGHT_THRESHOLD) return WorkLoad::MEDIUM;
            return WorkLoad::LIGHT;
         };

         //assign the job to an appropriate worker given the expected workload of the request
         _queueMtx.lock();
         switch (getWorkLoad(*newOrder)){
            case WorkLoad::LIGHT: _lightQueue.push(std::move(newOrder)); break;
            case WorkLoad::MEDIUM: _mediumQueue.push(std::move(newOrder)); break;
            case WorkLoad::HEAVY: _heavyQueue.push(std::move(newOrder)); break;
         }
         _requestCount++; //make sure this is always protected by the mutex
         _queueMtx.unlock();
         _condition_variable.notify_one();
         return {id, std::move(future)};
      }
   protected:
      static constexpr float LIGHT_THRESHOLD = 10;
      static constexpr float MEDIUM_THRESHOLD = 30;

      enum class WorkLoad {LIGHT, MEDIUM, HEAVY};

      struct WorkOrder{
         RequestId requestId;
         std::chrono::steady_clock::time_point requestTime;
         T* start;
         T* goal;
         float heuristic;
         std::promise<PathResult<T>> promise;
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
            _shutdown = true;
            if (_t.joinable()) _t.join();
         }
         void stop() { //call with the server's queue mutex held (see ~ReyStarServer)
            _shutdown = true;
         }
         void wait(){
            if (_t.joinable()) _t.join();
         }
         const int workerId;
      private:
         void work(){
            while (!_shutdown){
               std::unique_ptr<WorkOrder> order;
               std::unique_lock<std::mutex> lock(_server._queueMtx);
               _server._condition_variable.wait(lock, [&] {
                  // Wait for data. Predicate is protected by mutex during eval.
                  if (_shutdown) return true;
                  if (_server._requestCount) {
                     order = _server.getOpenWorkOrder();
                     return true;
                  }
                  return false;
               });
               lock.unlock();  // Release lock before processing
               if (!order) continue; //woken to shut down

               {
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
                  if (_server._finish) _server._finish(result);
                  order->promise.set_value(std::move(result));
               }
               //the order (and its promise) is freed here. The result lives on in the future's shared state.
            }
         }
         ReyStarServer& _server;
         std::thread _t;
         std::atomic<bool> _shutdown = false;
      };

      std::unique_ptr<WorkOrder> getOpenWorkOrder(){ //favors light then medium then heavy
         std::unique_ptr<WorkOrder> order;
         if (!_lightQueue.empty()) {
            order = std::move(_lightQueue.top());
            _lightQueue.pop();
         } else if (!_mediumQueue.empty()) {
            order = std::move(_mediumQueue.top());
            _mediumQueue.pop();
         } else if (!_heavyQueue.empty()) {
            order = std::move(_heavyQueue.top());
            _heavyQueue.pop();
         }
         _requestCount--;
         return order;
      }

      std::vector<Worker> _workers;

      std::mutex _queueMtx; //mutex for these queues
      std::condition_variable _condition_variable;

      using WorkOrderStack = std::stack<std::unique_ptr<WorkOrder>, std::vector<std::unique_ptr<WorkOrder>>>;
      WorkOrderStack _lightQueue; //light-ish requests
      WorkOrderStack _mediumQueue; //medium-ish requests
      WorkOrderStack _heavyQueue; //all others

      unsigned int _requestCount = 0; //must be protected by mutex

      RequestId _nextRequestId = 0;

      Tiles& tiles;
      const Finisher _finish; //set before the workers start, and never changed, so they can read it unguarded
      friend class Worker;
   };
   // so the tile type is worked out from the container: ReyStarServer server(tiles);
   template <ReyStarTileable T> ReyStarServer(TileMap<T>&) -> ReyStarServer<T, TileMap<T>>;
   template <ReyStarTileable T> ReyStarServer(Grid<T>&) -> ReyStarServer<T, Grid<T>>;
   template <ReyStarTileable T> ReyStarServer(TileMap<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>) -> ReyStarServer<T, TileMap<T>>;
   template <ReyStarTileable T> ReyStarServer(Grid<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>) -> ReyStarServer<T, Grid<T>>;
}
