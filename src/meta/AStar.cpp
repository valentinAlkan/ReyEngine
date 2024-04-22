#include "AStar.h"
#include "math.h"
#include <algorithm>
#include <queue>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D::AStar2D(const std::string& name)
: Component(name)
{
   data = make_unique<AStarData>();
   _t = std::thread(&AStar2D::run, this, std::ref(*data));
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
   data->requestShutdown = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
AStar2D& AStar2D::operator=(AStar2D&& other) noexcept {
   data = std::move(other.data);
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

inline double heuristic(const AStar2D::Cell& a, const AStar2D::Cell b) {
   return sqrt(pow(a.coordinates.x - b.coordinates.x, 2) + pow(a.coordinates.y - b.coordinates.y, 2));
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::run(AStarData& _data){

   auto& start = _data.start;
   auto& goal = _data.goal;
   auto& cancel = _data.cancel;
   auto& state = _data.state;
   auto& requestShutdown = _data.requestShutdown;
   auto& openSet = _data.openSet;
   auto& closedSet = _data.closedSet;
   auto& searchMtx = _data.searchMtx;
   auto& our_step = _data.our_step;
   auto& next_step = _data.next_step;
   auto& cv = _data.cv;
   while (!requestShutdown){
      switch (state){
         case SearchState::NO_SOLUTION:
            //todo: condtion variable?
            std::this_thread::sleep_for(100ms);
            if (start && goal) state = SearchState::READY;
            break;
         case SearchState::READY:
            //start a new search
            closedSet.clear();
            openSet.clear();
         case SearchState::SEARCHING:{
            //start with the first open cell
            auto& _goal = goal.value().get();
            cout << "Checking route between " << start->get() << " and " << _goal << endl;
            SearchNode startNode(start.value(), nullopt, 0, heuristic(start.value().get(), _goal));
            openSet.emplace_back(startNode);
            while (!openSet.empty() && !cancel) {
               std::unique_lock<std::mutex> lk(searchMtx);
               cv.wait(lk, [&]{return our_step == next_step; });
               auto currentNode = openSet.back();
               if (currentNode.cell.get() == _goal) break;
               auto currentHeuristic = currentNode.heuristic;
               for (auto& _neighbor : currentNode.cell.get()._connections){
                  auto nextHeuristic = heuristic(_neighbor.get(), _goal);
                  SearchNode node(_neighbor, currentNode, currentNode.pathWeight + _neighbor.get().weight, nextHeuristic);
                  //skip if this is in the closed set
                  auto it = closedSet.find(node);
                  if (it != closedSet.end()) continue;
                  if (nextHeuristic < currentHeuristic){
                     //the next path is more promising

                     openSet.emplace_back(node);
                  }
               }
               closedSet.emplace(currentNode);
               our_step++;
            }
            cout << " done ! " << endl;
            state = SearchState::FOUND;
            break;}
         case SearchState::FOUND:
            //set path here
            start.reset();
            goal.reset();
            state = SearchState::NO_SOLUTION;
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