// Headless checks of ReyStar: paths are cheapest (against a plain Dijkstra), and every way of calling it works.
// Exits non-zero on failure.
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <span>
#include "ReyStarServer.h"

using namespace ReyStar;
using ReyEngine::Pos;

namespace {
   int failures = 0;
   void check(bool ok, const char* what){
      if (!ok) {std::printf("FAIL: %s\n", what); failures++;}
   }
   bool near(float a, float b){return std::abs(a - b) <= 1e-4f * std::max(1.0f, std::abs(b));}

   /////////////////////////////////////////////////////////////////////////////////////////
   // 2D tiles, the way they've always been written

   // 4 neighbours in a vector, entering costs the weight, heuristic is manhattan
   struct MapTile {
      Pos<int> coord;
      float weight = 1;
      [[nodiscard]] std::vector<std::pair<int, int>> neighborCoords() const {
         return {{coord.x + 1, coord.y}, {coord.x - 1, coord.y}, {coord.x, coord.y + 1}, {coord.x, coord.y - 1}};
      }
      [[nodiscard]] float distanceTo(const MapTile& o) const {return (float)(std::abs(coord.x - o.coord.x) + std::abs(coord.y - o.coord.y));}
   };
   static_assert(ReyStarTileable<MapTile>);
   static_assert(TileContainer<TileMap<MapTile>, MapTile>);

   // 8 neighbours in an array, pricing its own steps from the search's parameters (see StepCosted, Estimated)
   struct Climb {float uphill = 1;};
   struct GridTile {
      Pos<int> coord;
      float weight = 1; //height
      [[nodiscard]] std::array<std::pair<int, int>, 8> neighborCoords() const {
         auto [x, y] = std::pair(coord.x, coord.y);
         return {{{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}, {x + 1, y + 1}, {x - 1, y - 1}, {x + 1, y - 1}, {x - 1, y + 1}}};
      }
      [[nodiscard]] float length(const GridTile& o) const {return std::hypot((float)(coord.x - o.coord.x), (float)(coord.y - o.coord.y));}
      [[nodiscard]] float distanceTo(const GridTile& o) const {return length(o);}
      [[nodiscard]] float stepCost(const GridTile& to, const Climb& p) const {return length(to) + p.uphill * std::max(0.0f, to.weight - weight);}
      [[nodiscard]] float estimate(const GridTile& goal, const Climb&) const {return length(goal);}
   };
   static_assert(ReyStarTileable<GridTile>);
   static_assert(StepCosted<GridTile, Climb> && Estimated<GridTile, Climb>);
   static_assert(TileContainer<Grid<GridTile>, GridTile>);

   /////////////////////////////////////////////////////////////////////////////////////////
   // graph nodes

   // scattered points, each joined to its nearest few, named by number in a Graph
   struct Point {
      float x = 0, y = 0;
      float weight = 1;
      std::vector<int> links;
      [[nodiscard]] std::span<const int> neighbors() const {return links;}
      [[nodiscard]] float length(const Point& o) const {return std::hypot(x - o.x, y - o.y);}
      [[nodiscard]] float distanceTo(const Point& o) const {return length(o);}
      [[nodiscard]] float stepCost(const Point& to, const NoParams&) const {return length(to) * to.weight;}
   };
   static_assert(ReyStarNode<Point> && !ReyStarTileable<Point>);
   static_assert(NodeContainer<Graph<Point>, Point> && SearchSpace<Graph<Point>, Point>);

   // a 2D tile that happens to have a neighbors() too: it must still be searched by its coordinates
   struct BothTile : MapTile {
      [[nodiscard]] std::array<int, 1> neighbors() const {return {-12345};}
   };
   static_assert(ReyStarTileable<BothTile> && ReyStarNode<BothTile>);
}

// a 3D grid in a container of its own, found by argument dependent lookup
namespace Voxels {
   struct Coord {
      int x, y, z;
      auto operator<=>(const Coord&) const = default;
   };
   struct Voxel {
      Coord at;
      float weight = 1;
      [[nodiscard]] std::array<Coord, 6> neighbors() const {
         auto [x, y, z] = at;
         return {{{x + 1, y, z}, {x - 1, y, z}, {x, y + 1, z}, {x, y - 1, z}, {x, y, z + 1}, {x, y, z - 1}}};
      }
      [[nodiscard]] float distanceTo(const Voxel& o) const {return (float)(std::abs(at.x - o.at.x) + std::abs(at.y - o.at.y) + std::abs(at.z - o.at.z));}
   };
   struct World {
      std::map<Coord, Voxel> voxels;
   };
   const Voxel* findTile(const World& world, const Coord& c){
      auto it = world.voxels.find(c);
      return it == world.voxels.end() ? nullptr : &it->second;
   }
   uint64_t tileKey(const World&, const Voxel& v){
      return ((uint64_t)(uint32_t)(v.at.x & 0x1fffff) << 42) | ((uint64_t)(uint32_t)(v.at.y & 0x1fffff) << 21) | (uint32_t)(v.at.z & 0x1fffff);
   }
}
static_assert(ReyStar::SearchSpace<Voxels::World, Voxels::Voxel>);

namespace {

   // Dijkstra over any tiles, given how to list a tile's neighbours and price a step
   template <typename T, typename Neighbors, typename Cost>
   float dijkstra(const T& start, const T& goal, Neighbors&& neighbors, Cost&& cost){
      std::map<const T*, float> dist{{&start, 0}};
      std::set<std::pair<float, const T*>> open{{0, &start}};
      while (!open.empty()) {
         auto [d, t] = *open.begin();
         open.erase(open.begin());
         if (t == &goal) return d;
         for (const T* n : neighbors(*t)) {
            float c = cost(*t, *n);
            if (!(c > 0) || std::isinf(c)) continue;
            auto it = dist.find(n);
            if (it != dist.end() && it->second <= d + c) continue;
            if (it != dist.end()) open.erase({it->second, n});
            dist[n] = d + c;
            open.insert({d + c, n});
         }
      }
      return -1;
   }

   template <typename Path, typename T>
   bool runsBetween(const Path& path, const T& start, const T& goal){
      return !path.empty() && path.front() == &start && path.back() == &goal;
   }

   void checkTileMap(){
      std::printf("TileMap\n");
      std::mt19937 rng(1);
      std::uniform_real_distribution<float> w(1, 4); //at least 1, so manhattan distance never guesses high
      TileMap<MapTile> tiles;
      for (int y = 0; y < 30; y++) {
         for (int x = 0; x < 30; x++) {
            if ((x % 7 == 3) && y > 2 && y < 27) continue; //holes: walls with gaps at the ends
            float weight = (x == 10 && y < 25) ? 0 : w(rng); //impassable
            tiles[{x, y}] = MapTile{{x, y}, weight};
         }
      }
      auto neighbors = [&](const MapTile& t){
         std::vector<const MapTile*> out;
         for (auto [x, y] : t.neighborCoords()) if (auto n = findTile(tiles, x, y)) out.push_back(n);
         return out;
      };
      auto cost = [](const MapTile&, const MapTile& to){return to.weight;};
      SearchScratch scratch;
      std::uniform_int_distribution<int> c(0, 29);
      int found = 0;
      for (int i = 0; i < 100; i++) {
         auto a = findTile(tiles, c(rng), c(rng));
         auto b = findTile(tiles, c(rng), c(rng));
         if (!a || !b || a->weight <= 0 || b->weight <= 0) continue;
         std::vector<const MapTile*> path;
         float cost1 = findPath<MapTile>(tiles, *a, *b, scratch, path);
         float expected = dijkstra(*a, *b, neighbors, cost);
         check(runsBetween(path, *a, *b), "TileMap: path runs from start to goal");
         check(near(cost1, expected), "TileMap: findPath finds a cheapest path");
         auto linked = aStarAlgorithm<MapTile>(tiles, *a, *b, scratch);
         size_t n = 0;
         for (auto& step : linked) {check(step.thisTile == path[n], "TileMap: aStarAlgorithm gives the same path"); n++;}
         check(n == path.size() && near(linked.pathCost, cost1), "TileMap: aStarAlgorithm gives the same path and cost");
         found++;
      }
      check(found > 20, "TileMap: enough searches ran");
      // no way through: a walled-in tile
      TileMap<MapTile> island;
      island[{0, 0}] = MapTile{{0, 0}, 1};
      island[{5, 5}] = MapTile{{5, 5}, 1};
      std::vector<const MapTile*> path;
      check(findPath<MapTile>(island, island[{0, 0}], island[{5, 5}], scratch, path) == 0 && path.empty(), "TileMap: no path");
      auto none = aStarAlgorithm<MapTile>(island, island[{0, 0}], island[{5, 5}]);
      check(none.begin() == none.end(), "TileMap: aStarAlgorithm with no path is empty");
   }

   void checkGrid(){
      std::printf("Grid\n");
      std::mt19937 rng(2);
      std::uniform_real_distribution<float> h(0, 5);
      Grid<GridTile> grid(41, 23);
      for (auto& t : grid) t.weight = h(rng);
      std::set<uint64_t> keys;
      for (auto& t : grid) keys.insert(tileKey(grid, t));
      check(keys.size() == grid.size(), "Grid: tileKey is unique");
      Climb climb{3};
      auto neighbors = [&](const GridTile& t){
         std::vector<const GridTile*> out;
         for (auto [x, y] : t.neighborCoords()) if (auto n = findTile(grid, x, y)) out.push_back(n);
         return out;
      };
      auto cost = [&](const GridTile& from, const GridTile& to){return from.stepCost(to, climb);};
      SearchScratch scratch;
      for (int i = 0; i < 100; i++) {
         auto& a = grid[rng() % grid.size()];
         auto& b = grid[rng() % grid.size()];
         std::vector<const GridTile*> path;
         float got = findPath<GridTile>(grid, a, b, climb, scratch, path);
         check(runsBetween(path, a, b), "Grid: path runs from start to goal");
         check(near(got, dijkstra(a, b, neighbors, cost)), "Grid: findPath with params finds a cheapest path");
      }
   }

   void checkServers(){
      std::printf("servers\n");
      // deduced from the container, with and without a finisher
      TileMap<MapTile> tiles;
      for (int y = 0; y < 20; y++) for (int x = 0; x < 20; x++) tiles[{x, y}] = MapTile{{x, y}, 1};
      {
         ReyStarServer server(tiles);
         auto result = server.requestPath(tiles[{0, 0}], tiles[{19, 19}]).get();
         check(result.pathFound && near(result.pathCost, 38) && result.path.size() == 39, "server(TileMap): finds the path");
         auto same = server.requestPath(tiles[{4, 4}], tiles[{4, 4}]).get();
         check(same.pathFound && same.path.size() == 1 && same.pathCost == 0, "server: start == goal");
      }
      {
         std::atomic<int> finished = 0;
         ReyStarServer server(tiles, [&](PathResult<MapTile>&){finished++;});
         auto [id, future] = server.requestPathAndReturnId(tiles[{0, 0}], tiles[{5, 0}]);
         auto result = future.get();
         check(result.requestId == id && near(result.pathCost, 5) && finished == 1, "server(TileMap, finisher): runs the finisher");
      }
      Grid<GridTile> grid(16, 16);
      {
         ReyStarServer server(grid, {}, 1 << 16);
         auto result = server.requestPath(grid.at(0, 0), grid.at(15, 0)).get();
         check(result.pathFound && near(result.pathCost, 15), "server(Grid, finisher, scratch): finds the path");
      }
      {
         // explicit params, a preparer and a finisher
         ReyStarServer<GridTile, Grid<GridTile>, Climb>::Preparer prepare = [](const GridTile&, const GridTile& goal, Climb& p, SearchScratch&){
            p.uphill = 2;
            return goal.coord.x != 13; //refuse anything to column 13
         };
         ReyStarServer<GridTile, Grid<GridTile>, Climb> server(grid, prepare, [](auto& r){r.pathCost += 1000;});
         grid.at(5, 0).weight = 4;
         std::vector<std::future<PathResult<GridTile, Climb>>> futures;
         for (int x = 1; x < 16; x++) futures.push_back(server.requestPath(grid.at(0, 0), grid.at(x, 0), Climb{1}));
         bool ok = true;
         for (auto& f : futures) {
            auto r = f.get();
            if (r.goal->coord.x == 13) {ok &= !r.pathFound; continue;}
            ok &= r.pathFound && r.params.uphill == 2 && r.pathCost > 1000;
         }
         check(ok, "server(Grid, params, preparer, finisher): many requests");
      }
   }
   void checkGraph(){
      std::printf("Graph\n");
      std::mt19937 rng(3);
      std::uniform_real_distribution<float> u(0, 100), w(1, 3);
      Graph<Point> graph(1500);
      for (auto& p : graph) {p.x = u(rng); p.y = u(rng); p.weight = w(rng);}
      for (size_t i = 0; i < graph.size(); i++) {
         std::vector<std::pair<float, int>> byDistance;
         for (size_t j = 0; j < graph.size(); j++) if (j != i) byDistance.push_back({graph[i].length(graph[j]), (int)j});
         std::partial_sort(byDistance.begin(), byDistance.begin() + 5, byDistance.end());
         for (int k = 0; k < 5; k++) graph[i].links.push_back(byDistance[k].second);
      }
      graph[7].links.push_back(999999); //a key with no node is skipped, as a coordinate with no tile is
      auto neighbors = [&](const Point& p){
         std::vector<const Point*> out;
         for (int id : p.links) if (auto n = graph.find(id)) out.push_back(n);
         return out;
      };
      auto cost = [](const Point& a, const Point& b){return a.stepCost(b, {});};
      SearchScratch scratch;
      int found = 0;
      for (int i = 0; i < 100; i++) {
         auto& a = graph[rng() % graph.size()];
         auto& b = graph[rng() % graph.size()];
         std::vector<const Point*> path;
         float got = findPath<Point>(graph, a, b, scratch, path);
         float expected = dijkstra(a, b, neighbors, cost);
         if (expected < 0) {check(path.empty(), "Graph: no path only when there's none"); continue;}
         found++;
         check(runsBetween(path, a, b), "Graph: path runs from start to goal");
         check(near(got, expected), "Graph: findPath finds a cheapest path");
      }
      check(found > 50, "Graph: enough paths found");
      auto linked = aStarAlgorithm<Point>(graph, graph[0], graph[1]);
      check(linked.begin() != linked.end() || dijkstra(graph[0], graph[1], neighbors, cost) < 0, "Graph: aStarAlgorithm");

      ReyStarServer server(graph);
      std::vector<std::future<PathResult<Point>>> futures;
      std::vector<float> expected;
      for (int i = 0; i < 40; i++) {
         auto& a = graph[rng() % graph.size()];
         auto& b = graph[rng() % graph.size()];
         futures.push_back(server.requestPath(a, b));
         expected.push_back(dijkstra(a, b, neighbors, cost));
      }
      bool ok = true;
      for (size_t i = 0; i < futures.size(); i++) {
         auto r = futures[i].get();
         ok &= expected[i] < 0 ? !r.pathFound : (r.pathFound && near(r.pathCost, expected[i]));
      }
      check(ok, "server(Graph): finds cheapest paths");
      auto same = server.requestPath(graph[3], graph[3]).get();
      check(same.pathFound && same.path.size() == 1, "server(Graph): start == goal");
   }

   void checkVoxels(){
      std::printf("3D\n");
      using namespace Voxels;
      World world;
      // a 12 cube with a wall at x = 6, open only at its top corner
      for (int z = 0; z < 12; z++) for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++) {
         bool wall = x == 6 && !(y == 11 && z == 11);
         world.voxels[{x, y, z}] = Voxel{{x, y, z}, wall ? 0.0f : 1.0f};
      }
      const Voxel& a = world.voxels.at({0, 0, 0});
      const Voxel& b = world.voxels.at({11, 0, 0});
      std::vector<const Voxel*> path;
      SearchScratch scratch;
      float got = findPath<Voxel>(world, a, b, scratch, path);
      check(runsBetween(path, a, b), "3D: path runs from start to goal");
      //to the gap at (6, 11, 11) is 6 + 11 + 11 steps, and from it 5 + 11 + 11 more
      if (!near(got, 55)) std::printf("  3D cost %g, %zu tiles\n", got, path.size());
      check(near(got, 55), "3D: path goes round the wall through the gap");
      bool throughGap = std::any_of(path.begin(), path.end(), [](const Voxel* v){return v->at.x == 6;});
      check(throughGap && path.size() == 56, "3D: path takes the gap");
      ReyStarServer<Voxel, World> server(world);
      auto r = server.requestPath(const_cast<Voxel&>(a), const_cast<Voxel&>(b)).get();
      check(r.pathFound && near(r.pathCost, got), "server(3D container)");
   }

   void checkBoth(){
      std::printf("2D tile with neighbors()\n");
      TileMap<BothTile> tiles;
      for (int x = 0; x < 10; x++) tiles[{x, 0}] = BothTile{{{x, 0}, 1}};
      std::vector<const BothTile*> path;
      SearchScratch scratch;
      float got = findPath<BothTile>(tiles, tiles[{0, 0}], tiles[{9, 0}], scratch, path);
      check(path.size() == 10 && near(got, 9), "searched by neighborCoords, not neighbors()");
   }
}

int main(){
   checkTileMap();
   checkGrid();
   checkServers();
   checkGraph();
   checkVoxels();
   checkBoth();
   std::printf(failures ? "%d FAILURES\n" : "all passed\n", failures);
   return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
