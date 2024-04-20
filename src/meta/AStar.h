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
   struct Cell{
   private:
      using CellRef = std::reference_wrapper<Cell>;
      struct CellCmp{
         bool operator()(const CellRef& lhs, const CellRef& rhs) const{
            return lhs.get().coordinates.x < rhs.get().coordinates.x ? false : lhs.get().coordinates.y < rhs.get().coordinates.y;
         }
      };
      using CellSet = std::set<std::reference_wrapper<Cell>, CellCmp>;
      CellSet _connections; //might have any number of connections
   public:
      Cell(const ReyEngine::Vec2<CoordinateType>& coordinates, double weight)
      : coordinates(coordinates)
      , weight(weight)
      {}
      ReyEngine::Vec2<CoordinateType> coordinates;
      double weight;
      const CellSet& getConnections() const {return _connections;}
      void addConnection(std::reference_wrapper<Cell>);
      void removeConnection(std::reference_wrapper<Cell>);
   };
   using Path = std::vector<Cell&>;
   /////////////////////////////////////////////////////////////////////////////////////////
   struct Connection{
//      double weight;
      Connection(Cell& a, Cell& b): a(a), b(b){}
      Cell& a;
      Cell& b;
   };
   struct Graph{
      std::optional<std::reference_wrapper<Cell>> getCell(const ReyEngine::Vec2<CoordinateType>&);
      void setCell(Cell&&);
   private:
      std::map<CoordinateType, std::map<CoordinateType, Cell>> _data;
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
   void setStart(Cell&);
   void setGoal(Cell&);
   void addConnection(Cell&, Cell&);
   void removeConnection(Connection&);
   std::optional<std::reference_wrapper<Cell>> getCell(ReyEngine::Vec2<CoordinateType>);
   Path& getPath();
private:
   static Cell NULL_CELL;
   void run(bool& requestShutdown, Graph& data);
   std::thread _t;
   std::unique_ptr<bool> _requestShutdown;
   std::unique_ptr<Graph> _graph;
   SearchState _state = SearchState::NO_SOLUTION;
   std::vector<std::reference_wrapper<Cell>> _path;
   std::optional<std::reference_wrapper<Cell>> _start;
   std::optional<std::reference_wrapper<Cell>> _goal;
};