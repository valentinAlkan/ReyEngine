#pragma once
#include "DrawInterface.h"
#include "Component.h"
#include <thread>
#include <chrono>
#include "Application.h"

//GraphOrder represents how many dimensions we will be working.
// A 2D map is secnd order, a 3D map is third order, and so on.
template <size_t GraphOrder, typename CoordinateType, typename WeightType>
class AStar : public Component {
   using Coordinates = std::array<CoordinateType, GraphOrder>;
private:
   struct Connection;
   struct Cell{
      Coordinates address;
      WeightType weight;
      std::vector<Connection*> connections; //might have any number of connections
   };
   struct Connection{
      Cell a;
      Cell b;
   };
   using Path = std::vector<Cell*>;
   struct Graph {
      std::array<CoordinateType, GraphOrder> data;
   };

public:
   AStar(const std::string& name = "Default")
   : Component(name)
   {
      _requestShutdown = std::make_unique<bool>();
      _data = std::make_unique<Graph>();
      _t = std::thread(&AStar::run, this, std::ref(*_requestShutdown), std::ref(*_data));
   }
   AStar(AStar&& other)
   : Component(other._name){
      (*this) = std::move(other);
   }
   AStar& operator=(AStar&& other){
      _requestShutdown = std::move(other._requestShutdown);
      _data = std::move(other._data);
      _t = std::move(other._t);
      return *this;
   }

   ~AStar(){
      if (_t.joinable()){
         shutdown();
         _t.join();
      }
   }
   void shutdown(){
      *_requestShutdown = true;
   }
private:
   void run(bool& requestShutdown, Graph& data) {
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
   std::thread _t;
   std::unique_ptr<bool> _requestShutdown;
   std::unique_ptr<Graph> _data;
};