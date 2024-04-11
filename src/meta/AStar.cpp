#include "AStar.h"

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
AStar2D::Cell& AStar2D::Graph::getCell(const Vec2<CoordinateType>& coords) {
   return _data.at(coords.x).at(coords.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::Graph::setCell(Cell&& cell) {
   _data[cell.coordinates.x].emplace(std::pair<CoordinateType, Cell>(cell.coordinates.y, std::move(cell)));
}

/////////////////////////////////////////////////////////////////////////////////////////
void AStar2D::run(bool &requestShutdown, Graph &data){
   using namespace std::chrono;
   std::this_thread::sleep_for(1000ms);
   std::cout << "AStar::run::start" << std::endl;
   while (!requestShutdown){
      static int i=0;
      std::this_thread::sleep_for(100ms);
      std::cout << "AStar::run::" << i++ << std::endl;
   }
   std::cout << "AStar::run::end " << std::endl;
}