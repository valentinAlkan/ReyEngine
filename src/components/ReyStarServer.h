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
#include <memory_resource>
#include <optional>
#include <ranges>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include "ReyEngine.h"

namespace ReyStar {
   // Any tile type usable with AStar must expose:
   //  - coord: its position
   //  - weight: nav cost of entering this tile (<= 0 means impassable)
   //  - neighborCoords(): adjacent coordinates - grid type and wrapping agnostic. Any range of (x, y) pairs will do; a
   //    fixed-size one (a std::array, say) saves the search a heap allocation for every tile it expands
   //  - distanceTo(other): heuristic
   // and may price steps itself (see StepCosted and Estimated).
   template <typename T>
   concept ReyStarTileable = requires(const T& t) {
      { t.coord } -> std::convertible_to<ReyEngine::Pos<int>>;
      { t.weight } -> std::convertible_to<float>;
      { t.neighborCoords() } -> std::ranges::input_range;
      requires std::convertible_to<std::ranges::range_value_t<decltype(t.neighborCoords())>, std::pair<int, int>>;
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

   /// Searches can be given parameters: whatever a tile needs to price a step that the tile itself doesn't hold (what
   /// is moving, say, or whether to save time or fuel). A search that isn't given any gets these.
   struct NoParams {};

   /// A tile type may price the steps off it itself, given a search's parameters, rather than the search charging the
   /// weight of the tile stepped onto: stepCost(to, params) is what stepping from this tile onto a neighbour costs. A
   /// step that costs 0 or less (or infinity, or isn't a number) can't be taken. A step may cost more one way than the
   /// other (uphill and down, say); the search only ever goes forwards.
   template <typename T, typename Params>
   concept StepCosted = requires(const T& from, const T& to, const Params& params) {
      { from.stepCost(to, params) } -> std::convertible_to<float>;
   };
   /// A tile type may likewise give the heuristic for a search's parameters: estimate(goal, params), a guess at what
   /// the cheapest path from this tile to the goal costs. To be sure of finding the cheapest path, it must never guess
   /// more. Without it, the search uses distanceTo.
   template <typename T, typename Params>
   concept Estimated = requires(const T& from, const T& goal, const Params& params) {
      { from.estimate(goal, params) } -> std::convertible_to<float>;
   };
   /// what a step from one tile onto a neighbour costs: as the tile prices it (see StepCosted), or the weight of the
   /// tile stepped onto
   template <typename T, typename Params>
   float stepCost(const T& from, const T& to, const Params& params) {
      if constexpr (StepCosted<T, Params>) return from.stepCost(to, params);
      else return to.weight;
   }
   /// the search's guess at what the cheapest path from a tile to the goal costs (see Estimated)
   template <typename T, typename Params>
   float estimate(const T& from, const T& goal, const Params& params) {
      if constexpr (Estimated<T, Params>) return from.estimate(goal, params);
      else return from.distanceTo(goal);
   }

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
   /// Memory for searches to work in, kept from one search to the next, so a search needn't go to the heap for what
   /// it learns about each tile. The heap can be slow when several threads use it at once (and on Windows, under a
   /// debugger, very slow), and a search otherwise asks it for memory tens of thousands of times.
   ///
   /// A search takes what it needs from the scratch's block, and gives it all back at once when it's done. If the
   /// block runs out, the search goes to the heap for more, in a few chunks that double in size; afterwards the block
   /// grows to what the search used, so the next search like it needn't. It never grows past keepAtMost bytes,
   /// so one huge search doesn't leave its memory held for good.
   ///
   /// One search at a time: each of the server's workers has its own.
   class SearchScratch {
   public:
      static constexpr size_t DEFAULT_KEEP = 16 << 20; //bytes
      explicit SearchScratch(size_t keepAtMost = DEFAULT_KEEP): _keepAtMost(keepAtMost){}
      SearchScratch(const SearchScratch&) = delete;
      SearchScratch& operator=(const SearchScratch&) = delete;
      SearchScratch(SearchScratch&&) = default;
      [[nodiscard]] size_t capacity() const {return _size;}
      /// While it lives, a search's memory comes from the scratch (see resource); when it goes, all of that memory is
      /// handed back. Whatever was made with the resource must be gone first.
      class Use {
      public:
         explicit Use(SearchScratch& scratch): _scratch(scratch) {
            if (_scratch._size) _resource.emplace(_scratch._block.get(), _scratch._size, &_upstream);
            else _resource.emplace(&_upstream);
         }
         ~Use() {
            _resource.reset();
            // it ran out: grow the block to what it used, so the next search as big stays inside it
            if (_upstream.used) {
               size_t wanted = std::min(_scratch._keepAtMost, _scratch._size + _upstream.used);
               if (wanted > _scratch._size) {
                  _scratch._block.reset(); //before allocating the new one, so the two are never held at once
                  _scratch._block = std::make_unique_for_overwrite<std::byte[]>(wanted);
                  _scratch._size = wanted;
               }
            }
         }
         Use(const Use&) = delete;
         Use& operator=(const Use&) = delete;
         [[nodiscard]] std::pmr::memory_resource* resource() {return &_resource.value();}
      private:
         /// the heap, counting what it hands out
         struct Upstream : std::pmr::memory_resource {
            size_t used = 0;
            void* do_allocate(size_t bytes, size_t alignment) override {
               used += bytes;
               return std::pmr::new_delete_resource()->allocate(bytes, alignment);
            }
            void do_deallocate(void* p, size_t bytes, size_t alignment) override {
               std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
            }
            bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {return this == &other;}
         };
         SearchScratch& _scratch;
         Upstream _upstream; //declared before _resource, which hands its chunks back to it as it goes
         std::optional<std::pmr::monotonic_buffer_resource> _resource;
      };
   private:
      std::unique_ptr<std::byte[]> _block;
      size_t _size = 0;
      size_t _keepAtMost;
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   /// Find the cheapest path from start to goal, with steps priced for these parameters (see StepCosted), using scratch
   /// for the search's memory (see SearchScratch). Fills path with the tiles from start to goal, both included, and
   /// gives what it costs; leaves path empty, and gives 0, if there's no way. The path is the only thing it takes from
   /// the heap: one block, sized to fit.
   template <ReyStarTileable T, typename Tiles, typename Params> requires TileContainer<Tiles, T>
   float findPath(const Tiles& tiles, const T& start, const T& goal, const Params& params, SearchScratch& scratch, std::vector<const T*>& path) {
      path.clear();
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

      // everything the search keeps comes from the scratch. Declared first, so it goes last, after what uses it.
      SearchScratch::Use memory(scratch);
      std::priority_queue<Node, std::pmr::vector<Node>, std::greater<Node>> openSet{std::greater<Node>(), std::pmr::vector<Node>(memory.resource())};
      std::pmr::unordered_map<uint64_t, Visit> visits(memory.resource());

      // Initialize with start node
      visits[tileKey(tiles, start)] = {0.0f};
      openSet.push({&start, 0.0f, estimate(start, goal, params)});

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
            // walk back from the goal to the start once to count the steps, then again to fill them in, back to front,
            // so the path is allocated once, at its size
            auto cameFrom = [&](const T* tile) {return visits.at(tileKey(tiles, *tile)).cameFrom;};
            size_t steps = 1;
            for (const T* tile = &goal; tile != &start && (tile = cameFrom(tile)); ) steps++;
            path.resize(steps);
            const T* tile = &goal;
            for (size_t i = steps; i-- > 0;) {
               path[i] = tile;
               if (i) tile = cameFrom(tile);
            }
            return current.gScore;
         }

         // Explore neighbors - adjacency is entirely up to the tile type
         for (const auto& [nx, ny] : current.tile->neighborCoords()) {
            const T* neighbor = findTile(tiles, nx, ny);
            if (!neighbor) {
               continue; // no tile at that coordinate
            }

            // Skip steps that can't be taken
            float cost = stepCost(*current.tile, *neighbor, params);
            if (!(cost > 0.0f) || std::isinf(cost)) {
               continue;
            }

            // Calculate tentative g-score
            float tentativeG = current.gScore + cost;

            // Check if this is a better path (one already expanded can't be: its cost is final)
            auto [it, isNew] = visits.try_emplace(tileKey(tiles, *neighbor), Visit{tentativeG, current.tile});
            if (!isNew) {
               if (it->second.closed || tentativeG >= it->second.gScore) {
                  continue;
               }
               it->second.gScore = tentativeG;
               it->second.cameFrom = current.tile;
            }
            float fScore = tentativeG + estimate(*neighbor, goal, params);
            openSet.push({neighbor, tentativeG, fScore});
         }
      }

      // No path found
      return 0;
   }

   /// Find the cheapest path from start to goal, as above, for a search with no parameters
   template <ReyStarTileable T, typename Tiles> requires TileContainer<Tiles, T>
   float findPath(const Tiles& tiles, const T& start, const T& goal, SearchScratch& scratch, std::vector<const T*>& path) {
      return findPath<T>(tiles, start, goal, NoParams{}, scratch, path);
   }

   /// Find the cheapest path from start to goal, using scratch for the search's memory (see SearchScratch), as a
   /// linked list of tiles. It has a node per tile, each from the heap; findPath gives the same path in one block.
   template <ReyStarTileable T, typename Tiles> requires TileContainer<Tiles, T>
   PathTile<T> aStarAlgorithm(const Tiles& tiles, const T& start, const T& goal, SearchScratch& scratch) {
      std::vector<const T*> path;
      float cost = findPath<T>(tiles, start, goal, scratch, path);
      if (path.empty()) return PathTile<T>{nullptr, nullptr}; //no path found
      // Build linked list from the end backwards
      std::unique_ptr<PathTile<T>> nextNode;
      for (auto it = path.rbegin(); it != path.rend(); ++it) {
         nextNode = std::make_unique<PathTile<T>>(*it, std::move(nextNode));
      }
      nextNode->pathCost = cost;
      return std::move(*nextNode);
   }

   /// Find the cheapest path from start to goal, with scratch memory of its own that goes when it's done.
   template <ReyStarTileable T, typename Tiles> requires TileContainer<Tiles, T>
   PathTile<T> aStarAlgorithm(const Tiles& tiles, const T& start, const T& goal) {
      SearchScratch scratch(0);
      return aStarAlgorithm<T>(tiles, start, goal, scratch);
   }

   /////////////////////////////////////////////////////////////////////////////////////////
   template <ReyStarTileable T, typename Params = NoParams>
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
         params = std::move(other.params);
         return *this;
      }
      RequestId requestId;
      std::chrono::microseconds processTime;
      bool pathFound;
      std::vector<const T*> path; //the tiles from start to goal, both included; empty if none was found
      float pathCost;
      T* start;
      T* goal;
      Params params; //what the search was asked to price steps for
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   /// Finds paths on worker threads (one per core) across a container of tiles: a TileMap (the default) or a Grid.
   /// The server keeps a reference to the container; its shape must not change while the server exists, but tiles'
   /// weights may (make them atomic if they change while searches run).
   ///
   /// Each request can carry parameters for pricing its steps (see StepCosted): a Params, copied into the request and
   /// handed back with its result.
   ///
   /// Each worker keeps scratch memory for its searches, up to scratchPerWorker bytes (see SearchScratch).
   ///
   /// It can be given a preparer: a function each request goes through on the worker, before the search, to do work
   /// the search needs that shouldn't land on the caller's thread (planning it roughly first, say, and narrowing the
   /// search to near that). It may change the request's parameters, which the search then uses, and has the worker's
   /// scratch memory to search with itself. It returns whether to search at all: if not, the request is answered with
   /// no path. Like the finisher, it may run on any worker at once, so it must be safe to call from several threads. A
   /// request whose start is its goal skips it.
   ///
   /// It can be given a finisher: a function each result goes through on the worker, after the search and before the
   /// future is ready, to do any work on the path that shouldn't land on the caller's thread (straightening it, say).
   /// It may change the result, path included, and may run on any worker at once, so it must be safe to call from
   /// several threads. A request whose start is its goal is answered straight away, so its result goes through the
   /// finisher on the caller's thread.
   template <ReyStarTileable T, typename Tiles = TileMap<T>, typename Params = NoParams> requires TileContainer<Tiles, T>
   class ReyStarServer {
   public:
      using Result = PathResult<T, Params>;
      using Finisher = std::function<void(Result&)>;
      using Preparer = std::function<bool(const T& start, const T& goal, Params& params, SearchScratch& scratch)>;
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
      explicit ReyStarServer(Tiles& tiles, Finisher finish = {}, size_t scratchPerWorker = SearchScratch::DEFAULT_KEEP)
      : ReyStarServer(tiles, Preparer{}, std::move(finish), scratchPerWorker)
      {}
      ReyStarServer(Tiles& tiles, Preparer prepare, Finisher finish, size_t scratchPerWorker = SearchScratch::DEFAULT_KEEP)
      : tiles(tiles)
      , _prepare(std::move(prepare))
      , _finish(std::move(finish))
      , _scratchPerWorker(scratchPerWorker)
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

      std::future<Result> requestPath(T& start, T& goal, const Params& params = {}) {
         return requestPathAndReturnId(start, goal, params).second;
      }

      std::pair<RequestId, std::future<Result>> requestPathAndReturnId(T& start, T& goal, const Params& params = {}) {
         //test to see if they are the same, if they are just short circuit
         auto id = _nextRequestId++;
         if (start.coord == goal.coord){
            Result path;
            path.pathFound = true;
            path.path = {&start};
            path.processTime = std::chrono::microseconds(0);
            path.requestId = id;
            path.pathCost = 0;
            path.start = &start;
            path.goal = &goal;
            path.params = params;
            if (_finish) _finish(path);
            auto promise = std::promise<Result>();
            promise.set_value(std::move(path));
            return {id, promise.get_future()};
         }

         //the order owns its promise and is handed from the queue to a worker, which frees it once it's answered
         auto newOrder = std::make_unique<WorkOrder>();
         newOrder->requestId = id;
         newOrder->requestTime = std::chrono::steady_clock::now();
         newOrder->start = &start;
         newOrder->goal = &goal;
         newOrder->params = params;
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
         Params params;
         float heuristic;
         std::promise<Result> promise;
      };

      struct Worker{
         Worker(int workerId, ReyStarServer& server)
         : workerId(workerId)
         , _server(server)
         , _scratch(server._scratchPerWorker)
         {
            _t = std::thread(&Worker::work, this);
         }
         Worker(const Worker&) = delete;
         Worker(Worker&& other)
         : workerId(other.workerId)
         , _server(other._server)
         , _scratch(std::move(other._scratch))
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
                  Result result;
                  bool search = !_server._prepare || _server._prepare(*order->start, *order->goal, order->params, _scratch);
                  result.pathCost = search ? findPath<T>(_server.tiles, *order->start, *order->goal, order->params, _scratch, result.path) : 0;
                  result.requestId = order->requestId;
                  result.pathFound = !result.path.empty();
                  result.processTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - order->requestTime);
                  result.start = order->start;
                  result.goal = order->goal;
                  result.params = std::move(order->params);
                  if (_server._finish) _server._finish(result);
                  order->promise.set_value(std::move(result));
               }
               //the order (and its promise) is freed here. The result lives on in the future's shared state.
            }
         }
         ReyStarServer& _server;
         SearchScratch _scratch; //only ever used on this worker's thread; declared before _t, so it's there before the thread starts
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
      const Preparer _prepare; //like _finish: set before the workers start, and never changed
      const Finisher _finish; //set before the workers start, and never changed, so they can read it unguarded
      const size_t _scratchPerWorker;
      friend class Worker;
   };
   // so the tile type is worked out from the container: ReyStarServer server(tiles);
   template <ReyStarTileable T> ReyStarServer(TileMap<T>&) -> ReyStarServer<T, TileMap<T>>;
   template <ReyStarTileable T> ReyStarServer(Grid<T>&) -> ReyStarServer<T, Grid<T>>;
   template <ReyStarTileable T> ReyStarServer(TileMap<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>) -> ReyStarServer<T, TileMap<T>>;
   template <ReyStarTileable T> ReyStarServer(Grid<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>) -> ReyStarServer<T, Grid<T>>;
   template <ReyStarTileable T> ReyStarServer(TileMap<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>, size_t) -> ReyStarServer<T, TileMap<T>>;
   template <ReyStarTileable T> ReyStarServer(Grid<T>&, std::type_identity_t<std::function<void(PathResult<T>&)>>, size_t) -> ReyStarServer<T, Grid<T>>;
}
