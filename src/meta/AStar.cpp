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
   return _data.at(coords.x).at(coords.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Graph::setCell(Cell&& cell) {
   _data[cell.coordinates.x].emplace(std::pair<CoordinateType, Cell>(cell.coordinates.y, std::move(cell)));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::reference_wrapper<AStar2D::Cell>> AStar2D::getCell(ReyEngine::Vec2<CoordinateType> coords) {
   return _graph->getCell(coords);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::setStart(Cell& start) {_start = start;}
void AStar2D::setGoal(Cell& goal) {_goal = goal;}
void AStar2D::addConnection(Cell& a, Cell& b) {
   a.addConnection(b);
}
void AStar2D::removeConnection(Connection &) {}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::run(bool &requestShutdown, Graph &data){
   std::cout << "AStar::run::start" << std::endl;
   std::vector<std::reference_wrapper<Cell>> openList;
   std::vector<std::reference_wrapper<Cell>> closedList;

   auto generateHeuristic = [](Cell& start, Cell& goal){
      //use euclidian distance
      double x = start.coordinates.x - goal.coordinates.x;
      double y = start.coordinates.y - goal.coordinates.y;
      return sqrt(pow(x, 2) + pow(y, 2));
   };

   while (!requestShutdown){
      switch (_state){
         case SearchState::NO_SOLUTION:
            //todo: condtion variable?
            std::this_thread::sleep_for(100ms);
            if (_start && _goal) _state = SearchState::READY;
            break;
         case SearchState::READY:
            //start a new search
            openList.clear();
            closedList.clear();
         case SearchState::SEARCHING:
            if (!_start || !_goal) {_state = SearchState::NO_SOLUTION; continue;}
            //start with the first opencell
            openList.push_back(_start.value());
            break;
         case SearchState::FOUND:
            break;
      }
   }
   std::cout << "AStar::run::end " << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Cell::addConnection(std::reference_wrapper<Cell> neighbor) {
   _connections.insert(neighbor);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Cell::removeConnection(std::reference_wrapper<Cell> neighbor) {
   auto found = _connections.find(neighbor);
   if (found != _connections.end()){
      _connections.erase(found);
   }
}