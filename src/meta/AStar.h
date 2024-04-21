#pragma once
#include "DrawInterface.h"
#include "Component.h"
#include <thread>
#include <chrono>
#include "Application.h"
#include "MultiDimensionalArray.h"
#include <set>
//int main()
//{
//   array<int, 2, 3>::type a4 = { { 1, 2, 3}, { 1, 2, 3} };
//   array<int, 2, 3> a5{ { { 1, 2, 3}, { 4, 5, 6} } };
//   std::cout << a5[1][2] << std::endl;
//
//   array<int, 3> a6{ {1, 2, 3} };
//   std::cout << a6[1] << std::endl;
//
//   array<int, 1, 2, 3> a7{ { { { 1, 2, 3}, { 4, 5, 6 } } }};
//   std::cout << a7[0][1][2] << std::endl;
//}

//GraphOrder represents how many dimensions we will be working.
// A 2D map is secnd order, a 3D map is third order, and so on.
class AStar2D : public Component {
public:
   /////////////////////////////////////////////////////////////////////////////////////////
   using CoordinateType = int;
   using Heuristic = float;
   class Graph;
   struct Cell{
   private:
      using CellRef = std::reference_wrapper<Cell>;
      struct CellCmp{
         bool operator()(const CellRef& lhs, const CellRef& rhs) const{
//            If comp(a, b) == true then comp(b, a) == false.
//            if comp(a, b) == true and comp(b, c) == true then comp(a, c) == true.
            return lhs.get().coordinates.x < rhs.get().coordinates.x || lhs.get().coordinates.y < rhs.get().coordinates.y;
         }
      };
      using CellSet = std::set<std::reference_wrapper<Cell>, CellCmp>;
      CellSet _connections; //might have any number of (unique) connections (no repeats!!!)
   protected:
      Cell(const ReyEngine::Vec2<CoordinateType>& coordinates, double weight)
            : coordinates(coordinates)
            , weight(weight)
      {}
//      Cell(Cell&& other) noexcept {
//         weight = other.weight;
//         coordinates = other.coordinates;
//         _connections = std::move(other._connections);
//      }
   public:
      ReyEngine::Vec2<CoordinateType> coordinates;
      double weight;
      [[nodiscard]] const CellSet& getConnections() const {return _connections;}
      void connect(std::reference_wrapper<Cell>);
      void disconnect(std::reference_wrapper<Cell>);
      inline bool operator==(const Cell& other){return coordinates == other.coordinates;}
      inline bool operator!=(const Cell& other){return coordinates != other.coordinates;}
      friend class Graph;
   };
   using Path = std::vector<Cell&>;
   /////////////////////////////////////////////////////////////////////////////////////////
   struct Connection{
      Connection(Cell& a, Cell& b): a(a), b(b){}
      Cell& a;
      Cell& b;
   };
   struct Graph{
      std::optional<std::reference_wrapper<Cell>> getCell(const ReyEngine::Vec2<CoordinateType>&);
//      void setCell(Cell&&);
      AStar2D::Cell& createCell(const ReyEngine::Vec2<int>&, double weight);
      std::optional<std::reference_wrapper<Cell>> start;
      std::optional<std::reference_wrapper<Cell>> goal;
      bool cancel = false;

   private:
      std::map<CoordinateType, std::map<CoordinateType, Cell>> _data;



      // Iterator class for rows of csv data
//      class iterator : public std::iterator<std::forward_iterator_tag, std::string> {
//      public:
//         iterator(std::optional<std::reference_wrapper<Graph>> graph = std::nullopt)
//         : _parser(parser)
//         {}
//         const Row& operator*() const {
//            auto& r = _parser.value().get()._data.at(rowNo);
//            return r;
//         }
//         iterator& operator++() {
//            rowNo++;
//            return *this;
//         }
//
//         bool operator!=(const iterator& other) const {
//            if (!_parser) return false;
//            if (!other._parser) {return rowNo < _parser.value().get().getAllRows().size();}
//            return _parser.value().get()._data[rowNo] != other._parser.value().get()._data[rowNo];
//         }
//
//         size_t getCurrentRowNo(){return rowNo;}
//      private:
//         size_t rowNo = 0;
//         std::optional<std::reference_wrapper<CSVParser>> _parser;
//      };
//
//
//      iterator begin() {
//         auto it = iterator(std::ref(*this));
//         return it;
//      }
//      iterator end() const { return {};}


   };
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class SearchState {
      NO_SOLUTION = 0,
      READY,
      SEARCHING,
      FOUND
   };
   AStar2D(const std::string& name = "Default");
   AStar2D(AStar2D&& other);
   AStar2D& operator=(AStar2D& other) = delete;
   AStar2D& operator=(AStar2D&& other) noexcept;
   std::unique_ptr<Graph>& getGraph(){return _graph;}
   ~AStar2D();
   void shutdown();
   SearchState getSearchState(){return _state;}
   void setStart(Cell& start){_graph->start = start;}
   void setGoal(Cell& goal){_graph->goal = goal;}
//   std::optional<std::reference_wrapper<Cell>> getCell(ReyEngine::Vec2<CoordinateType>);
   Path& getPath();
private:
   static Cell NULL_CELL;
   void run(bool& requestShutdown, Graph& data);
   std::thread _t;
   std::unique_ptr<bool> _requestShutdown;
   std::unique_ptr<Graph> _graph;
   SearchState _state = SearchState::NO_SOLUTION;
   std::vector<std::reference_wrapper<Cell>> _path;
};