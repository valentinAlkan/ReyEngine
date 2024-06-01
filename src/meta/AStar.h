//#pragma once
//#include "DrawInterface.h"
//#include "Component.h"
//#include <thread>
//#include <chrono>
//#include "Application.h"
//#include "MultiDimensionalArray.h"
//#include <set>
//#include <condition_variable>
//
////int main()
////{
////   array<int, 2, 3>::type a4 = { { 1, 2, 3}, { 1, 2, 3} };
////   array<int, 2, 3> a5{ { { 1, 2, 3}, { 4, 5, 6} } };
////   std::cout << a5[1][2] << std::endl;
////
////   array<int, 3> a6{ {1, 2, 3} };
////   std::cout << a6[1] << std::endl;
////
////   array<int, 1, 2, 3> a7{ { { { 1, 2, 3}, { 4, 5, 6 } } }};
////   std::cout << a7[0][1][2] << std::endl;
////}
//
////GraphOrder represents how many dimensions we will be working.
//// A 2D map is secnd order, a 3D map is third order, and so on.
//class AStar2D : public Component {
//public:
//   /////////////////////////////////////////////////////////////////////////////////////////
//   using CoordinateType = int;
//   using Heuristic = double;
//   class Graph;
//   struct Cell{
//      //adapter structs for different containers
//      using CellRef = std::reference_wrapper<Cell>;
//      using CostPair = std::pair<CellRef, double>;
//      struct CellLess {
//         inline bool operator()(const CellRef lhs, const CellRef rhs) const {
//            return lhs.get().coordinates.x < rhs.get().coordinates.x || lhs.get().coordinates.y < rhs.get().coordinates.y;
//         }
//      };
//      struct CellHash{
//         inline std::size_t operator()(const CellRef& c) const noexcept {
//            size_t bigboy = c.get().coordinates.x;
//            bigboy <<= (sizeof(int));
//            bigboy &= c.get().coordinates.y;
//            return std::hash<size_t>{}(bigboy);
//         }
//      };
//      struct CellEqual {
//         inline bool operator()(const CellRef& lhs, const CellRef& rhs) const noexcept {
//            return lhs.get() == rhs.get();
//         }
//      };
//      struct WeightCompare {
//         inline bool operator()(const CostPair lhs, const CostPair rhs) const {
//            auto lhsTotal = lhs.first.get().weight + lhs.second;
//            auto rhsTotal = rhs.first.get().weight + rhs.second;
//            return lhsTotal < rhsTotal;
//         }
//         inline bool operator()(const CellRef lhs, const CellRef rhs) const {
//            return lhs.get().weight < rhs.get().weight;
//         }
//      };
//      using CellSet = std::set<CellRef, CellLess>;
//      CellSet _connections; //might have any number of (unique) connections (no repeats!!!)
//   protected:
//      Cell(const ReyEngine::Vec2<CoordinateType>& coordinates, double weight)
//            : coordinates(coordinates)
//            , weight(weight)
//      {}
////      Cell(Cell&& other) noexcept {
////         weight = other.weight;
////         coordinates = other.coordinates;
////         _connections = std::move(other._connections);
////      }
//   public:
//      ReyEngine::Vec2<CoordinateType> coordinates;
//      double weight;
//      [[nodiscard]] const CellSet& getConnections() const {return _connections;}
//      void connect(CellRef);
//      void disconnect(CellRef);
//      inline bool operator==(const Cell& other){return coordinates == other.coordinates;}
//      inline bool operator!=(const Cell& other){return coordinates != other.coordinates;}
//      inline bool operator<(const Cell& other) {
//         return coordinates.x < other.coordinates.x;
//      }
//      friend std::ostream& operator<<(std::ostream& os, const Cell& c) {os << c.coordinates; return os;}
//      friend class Graph;
//   };
//   using Path = std::vector<Cell&>;
//   /////////////////////////////////////////////////////////////////////////////////////////
//   struct Connection{
//      Connection(Cell& a, Cell& b): a(a), b(b){}
//      Cell& a;
//      Cell& b;
//   };
//   struct Graph{
//      std::optional<Cell::CellRef> getCell(const ReyEngine::Vec2<CoordinateType>&);
////      void setCell(Cell&&);
//      AStar2D::Cell& createCell(const ReyEngine::Vec2<int>&, double weight);
//      size_t size(){return _graphSize;}
//   private:
//      std::map<CoordinateType, std::map<CoordinateType, Cell>> _data;
//      size_t _graphSize = 0;
//   };
//   /////////////////////////////////////////////////////////////////////////////////////////
//   enum class SearchState {
//      NO_SOLUTION = 0,
//      READY,
//      SEARCHING,
//      CANCELLED,
//      FOUND
//   };
//   AStar2D(const std::string& name = "Default");
//   AStar2D(AStar2D&& other);
//   AStar2D& operator=(AStar2D& other) = delete;
//   AStar2D& operator=(AStar2D&& other) noexcept;
//   Graph& getGraph(){return data->graph;}
//   ~AStar2D();
//   void shutdown();
//   SearchState getSearchState(){return data->state;}
//   void setStart(Cell& start){data->start = start;}
//   void setGoal(Cell& goal){data->goal = goal;}
//   void cancel(){data->state = SearchState::CANCELLED;}
//   void clearStart(){data->start = std::nullopt;}
//   void clearGoal(){data->goal = std::nullopt;}
//   Path& getPath();
//
//   void setNextStep() {
//      data->doNext = true;
//      data->cv.notify_all();
//
//   }
//private:
//
//
//   struct SearchNode {
//      SearchNode(const Cell::CellRef& cell, std::optional<std::reference_wrapper<std::unique_ptr<SearchNode>>> parent, double pathWeight, double heuristic)
//      : cell(cell)
//      , parent(parent)
//      , pathWeight(pathWeight)
//      , heuristic(heuristic){}
//      Cell::CellRef cell;
//      std::optional<std::reference_wrapper<std::unique_ptr<SearchNode>>> parent;
//      double pathWeight;
//      double heuristic;
//
//      struct WeightComp {
//         inline bool operator()(const std::reference_wrapper<SearchNode> &lhs, const std::reference_wrapper<SearchNode> &rhs) const {
//            return lhs.get().pathWeight < rhs.get().pathWeight;
//         }
//      };
//      struct NodeHash {
//         inline std::size_t operator()(const std::unique_ptr<SearchNode> &n) const noexcept {
//            return Cell::CellHash()(n->cell);
//         }
//      };
//      struct NodeLess{
//         inline std::size_t operator()(const std::unique_ptr<SearchNode>& lhs, const std::unique_ptr<SearchNode>& rhs) const noexcept {
//            return Cell::CellLess()(lhs->cell, rhs->cell);
//         }
//      };
//      struct NodeEqual{
//         bool operator()(const std::unique_ptr<SearchNode>& lhs, const std::unique_ptr<SearchNode>& rhs) const{
//            return lhs->cell.get() == rhs->cell.get();
//         }
//      };
//   };
//
//
//
//
//   using CellRef = Cell::CellRef;
//   using CostPair = Cell::CostPair;
//   using NodeRef = std::reference_wrapper<SearchNode>;
//
//
//
//   using OpenSet = std::vector<std::unique_ptr<SearchNode>>;
//   using ClosedSet = std::unordered_set<std::unique_ptr<SearchNode>, SearchNode::NodeHash, SearchNode::NodeEqual>;
////   using OpenSet = std::priority_queue<NodeRef, std::vector<NodeRef>, SearchNode::WeightComp>;
////   using ClosedSet = std::vector<NodeRef>;
//
//
//
//   static Cell NULL_CELL;
//   std::thread _t;
//
//public:
//   using SetLock = std::unique_ptr<std::scoped_lock<std::mutex>>;
//   std::pair<ClosedSet&, SetLock> getClosedSet(){
//      std::pair<ClosedSet&, SetLock> p(data->closedSet, std::make_unique<std::scoped_lock<std::mutex>>(data->closedSetMtx));
//      return p;
//   }
//   std::pair<OpenSet&, SetLock> getOpenSet(){
//      std::pair<OpenSet&, SetLock> p(data->openSet, std::make_unique<std::scoped_lock<std::mutex>>(data->closedSetMtx));
//      return p;
//   }
//
//private:
//   struct AStarData{
//      Graph graph;
//      bool requestShutdown = false;
//      std::optional<CellRef> start;
//      std::optional<CellRef> goal;
//      bool doNext = false;
//      std::condition_variable cv;
//      SearchState state = SearchState::NO_SOLUTION;
//      std::vector<Cell::CellRef> path;
//      std::mutex pathMtx;
//      std::mutex searchMtx;
//      std::mutex openSetMtx;
//      std::mutex closedSetMtx;
//      ClosedSet closedSet;
//      OpenSet openSet;
//   };
//
//   std::unique_ptr<AStarData> data;
//
//public:
//   void run(AStarData& data);
//};
//
//
//
////for hashing
////namespace std {
////   template<>
////   struct hash<AStar2D::Cell::CellRef> {
////      size_t operator()(const AStar2D::Cell::CellRef& c) const {
////         size_t bigboy = c.get().coordinates.x;
////         bigboy <<= (sizeof(int));
////         bigboy &= c.get().coordinates.y;
////         return std::hash<size_t>{}(bigboy);
////      }
////   };
////}