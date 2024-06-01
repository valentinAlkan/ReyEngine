//#include "AStar.h"
//#include "math.h"
//#include <algorithm>
//#include <queue>
//
//using namespace std;
//using namespace ReyEngine;
//
///////////////////////////////////////////////////////////////////////////////////////////
//AStar2D::AStar2D(const std::string& name)
//: Component(name)
//{
//   data = make_unique<AStarData>();
//   _t = std::thread(&AStar2D::run, this, std::ref(*data));
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//AStar2D::AStar2D(AStar2D&& other)
//: Component(other._name){
//   (*this) = std::move(other);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//AStar2D::~AStar2D(){
//   if (_t.joinable()){
//      shutdown();
//      _t.join();
//   }
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::shutdown() {
//   data->requestShutdown = true;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//AStar2D& AStar2D::operator=(AStar2D&& other) noexcept {
//   data = std::move(other.data);
//   _t = std::move(other._t);
//   return *this;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//std::optional<std::reference_wrapper<AStar2D::Cell>> AStar2D::Graph::getCell(const Vec2<CoordinateType>& coords) {
//   try{
//      return _data.at(coords.x).at(coords.y);
//   } catch (...){}
//   return nullopt;
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//AStar2D::Cell& AStar2D::Graph::createCell(const ReyEngine::Vec2<int>& coordinates, double weight) {
//   _data[coordinates.x].emplace(std::pair<CoordinateType, Cell>(coordinates.y, Cell(coordinates, weight)));
//   auto ref = getCell(coordinates); //gauranteed to exist
//   _graphSize++;
//   return ref.value();
//}
//
/////////////////////////////////////////////////////////////////////////////////////////////
//inline double heuristic(const AStar2D::Cell& a, const AStar2D::Cell b) {
//   return sqrt(pow(a.coordinates.x - b.coordinates.x, 2) + pow(a.coordinates.y - b.coordinates.y, 2));
//}
//inline double heuristicManhattan(const AStar2D::Cell& a, const AStar2D::Cell b) {
//   return abs(a.coordinates.x - b.coordinates.x) + abs(a.coordinates.y - b.coordinates.y);
//}
//
//
///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::run(AStarData& _data){
//
//   auto& start = _data.start;
//   auto& goal = _data.goal;
//   auto& state = _data.state;
//   auto& requestShutdown = _data.requestShutdown;
//   auto& openSet = _data.openSet;
//   auto& closedSet = _data.closedSet;
//   auto& searchMtx = _data.searchMtx;
//   auto& doNext = _data.doNext;
//   auto& cv = _data.cv;
//   openSet.reserve(_data.graph.size()); //prereserve enough cells so we don't invalidate our iterators
//   while (!requestShutdown){
//      switch (state){
//         case SearchState::NO_SOLUTION:
//            //todo: condtion variable?
//            std::this_thread::sleep_for(100ms);
//            if (start && goal) state = SearchState::READY;
//            break;
//         case SearchState::READY:{
//            //start a new search
//            closedSet.clear();
//            openSet.clear();
//            //start with the first open cell
//            auto& _goal = goal.value().get();
//            cout << "Checking route between " << start->get() << " and " << _goal << endl;
//            auto startNode = make_unique<SearchNode> (start.value(), nullopt, 0, heuristic(start.value().get(), _goal));
//            openSet.emplace_back(std::move(startNode));
//            state = SearchState::SEARCHING;
//            break;}
//         case SearchState::SEARCHING:{
//            auto& _goal = goal.value().get();
//            bool found = false;
//            while (!openSet.empty() && state != SearchState::CANCELLED) {
//               std::unique_lock<std::mutex> lk(searchMtx);
//               cv.wait(lk, [&]{return doNext; });
//               scoped_lock<mutex> sl1(_data.openSetMtx);
//               scoped_lock<mutex> sl2(_data.closedSetMtx);
//               if ((*openSet.begin())->cell.get() == _goal) break;
//               auto currentHeuristic = (*openSet.begin())->heuristic;
//               for (auto& _neighbor : (*openSet.begin())->cell.get()._connections){
//                  auto& currentNode = *openSet.begin();
//                  if (_goal == _neighbor){
//                     found = true;
//                     break;
//                  };
//                  auto nextHeuristic = heuristic(_neighbor.get(), _goal);
//                  auto node = make_unique<SearchNode>(_neighbor, currentNode, currentNode->pathWeight + _neighbor.get().weight, nextHeuristic);
//                  //skip if this is in the closed set
//                  auto it = closedSet.find(node);
//                  if (it != closedSet.end()){
//                     //we already searched this node. remove it from the open set
////                     openSet.erase(openSet.begin());
//                     continue;
//                  };
//                  if (nextHeuristic < currentHeuristic){
//                     //the next path is more promising
//                     openSet.emplace_back(std::move(node));
//                     cout << "adding to open set. Size is now " << openSet.size() << endl;
//                  }
//               }
//               if (found) break;
//               closedSet.emplace(std::move(*openSet.begin()));
//               openSet.erase(openSet.begin());
////               openSet.erase(openSet.begin());
//               doNext = false;
//            }
//            cout << " done ! " << endl;
//            state = SearchState::FOUND;
//            break;}
//         case SearchState::FOUND:
//            //set path here
//            start.reset();
//            goal.reset();
//            openSet.clear();
//            closedSet.clear();
//            state = SearchState::NO_SOLUTION;
//            break;
//      }
//   }
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::Cell::connect(std::reference_wrapper<Cell> neighbor) {
//   _connections.insert(neighbor);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void AStar2D::Cell::disconnect(std::reference_wrapper<Cell> neighbor) {
//   auto found = _connections.find(neighbor);
//   if (found != _connections.end()){
//      _connections.erase(found);
//   }
//}