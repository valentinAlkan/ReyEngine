#include "AStar.h"
#include "math.h"
#include <algorithm>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D::AStar2D(const std::string& name)
: Component(name)
{
   _requestShutdown = std::make_unique<bool>();
   _graph = std::make_unique<Graph>();
   _t = std::thread(&AStar2D::run, this, std::ref(*_requestShutdown), std::ref(*_graph));
}

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D::AStar2D(AStar2D&& other)
: Component(other._name){
   (*this) = std::move(other);
}

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D::~AStar2D(){
   if (_t.joinable()){
      shutdown();
      _t.join();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::shutdown() {
   *_requestShutdown = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D& AStar2D::operator=(AStar2D&& other) noexcept {
   _requestShutdown = std::move(other._requestShutdown);
   _graph = std::move(other._graph);
   _t = std::move(other._t);
   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::reference_wrapper<AStar2D::Cell>> AStar2D::Graph::getCell(const Vec2<CoordinateType>& coords) {
   try{
      return _data.at(coords.x).at(coords.y);
   } catch (...){}
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
AStar2D::Cell& AStar2D::Graph::createCell(const ReyEngine::Vec2<int>& coordinates, double weight) {
   _data[coordinates.x].emplace(std::pair<CoordinateType, Cell>(coordinates.y, Cell(coordinates, weight)));
   auto ref = getCell(coordinates); //gauranteed to exist
   return ref.value();
}

///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::Graph::setCell(Cell&& cell) {
//   _data[cell.coordinates.x].emplace(std::pair<CoordinateType, Cell>(cell.coordinates.y, std::move(cell)));
//}

///////////////////////////////////////////////////////////////////////////////////////////
//std::optional<std::reference_wrapper<AStar2D::Cell>> AStar2D::getCell(ReyEngine::Vec2<CoordinateType> coords) {
//
//}

///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::Graph::addConnection(Cell& a, Cell& b) {
//   a.addConnection(b);
//}
//void AStar2D::Graph::removeConnection(Connection &) {
//   //todo:
//   assert(false);
//}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::run(bool &requestShutdown, Graph &data){
   std::vector<std::reference_wrapper<Cell>> closedList;

   auto generateHeuristic = [](Cell& start, Cell& goal) -> double{
      //use euclidian distance
      double x = start.coordinates.x - goal.coordinates.x;
      double y = start.coordinates.y - goal.coordinates.y;
      auto exp = pow(x, 2) + pow(y, 2);
      auto squirt = sqrt(exp);
      return squirt;
   };

   auto& start = data.start;
   auto& goal = data.goal;
   auto& cancel = data.cancel;
   while (!requestShutdown){
      switch (_state){
         case SearchState::NO_SOLUTION:
            //todo: condtion variable?
            std::this_thread::sleep_for(100ms);
            if (start && goal) _state = SearchState::READY;
            break;
         case SearchState::READY:
            //start a new search
            closedList.clear();
         case SearchState::SEARCHING:{
            //start with the first open cell
            auto& _goal = goal.value().get();
            auto openCell = start.value();
            //generate the heuristic
            while(!cancel || openCell.get() != _goal) {
               auto heuristic = generateHeuristic(openCell, goal->get());
               //see if any of the connected cells have better heuristics
               for (const auto &connectedCell: openCell.get().getConnections()) {
                  cout << "checking connection between " << connectedCell.get().coordinates << " & " << _goal.coordinates << endl;
                  auto newHeuristic = generateHeuristic(connectedCell.get(), _goal);
                  if (newHeuristic < heuristic) {
                     //this is a better option than what we currently have - close the cell and make a new open cell
                     closedList.emplace_back(openCell);
                     openCell = connectedCell;
                     heuristic = newHeuristic;
                     break;
                  }
               }
            }
            //if we make it this far, we are next to the goal
            closedList.emplace_back(openCell);
            closedList.emplace_back(_goal);
            _state = SearchState::FOUND;
            if (cancel) _state = SearchState::NO_SOLUTION;
            /* valid path in closed list */
            break;}
         case SearchState::FOUND:
            break;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Cell::connect(std::reference_wrapper<Cell> neighbor) {
   _connections.insert(neighbor);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Cell::disconnect(std::reference_wrapper<Cell> neighbor) {
   auto found = _connections.find(neighbor);
   if (found != _connections.end()){
      _connections.erase(found);
   }
}