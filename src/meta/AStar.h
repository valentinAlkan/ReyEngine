#pragma once
#include "DrawInterface.h"
#include "Component.h"
#include <thread>
#include <chrono>
#include "Application.h"
#include "MultiDimensionalArray.h"

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
template <size_t GraphOrder, typename CoordinateType, typename WeightType>
class AStar : public Component {
private:
   /////////////////////////////////////////////////////////////////////////////////////////
   using Coordinates = std::array<CoordinateType, GraphOrder>;
   struct Connection;
   struct Cell{
      Coordinates address;
      WeightType weight;
      std::vector<std::unique_ptr<Connection>> connections; //might have any number of connections
   };
   using Path = std::vector<Cell*>;
   /////////////////////////////////////////////////////////////////////////////////////////
   struct Connection{
      Cell a;
      Cell b;
   };
   /////////////////////////////////////////////////////////////////////////////////////////

   using Graph = MultiDimensionalArray<Cell, GraphOrder>;

public:
   AStar(const std::string& name = "Default")
   : Component(name)
   {
      _requestShutdown = std::make_unique<bool>();
      _data = std::make_unique<Graph>();
      _t = std::thread(&AStar::run, this, std::ref(*_requestShutdown), std::ref(*_data));
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   AStar(AStar&& other)
   : Component(other._name){
      (*this) = std::move(other);
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   AStar& operator=(AStar& other) = delete;
   AStar& operator=(AStar&& other) noexcept {
      _requestShutdown = std::move(other._requestShutdown);
      _data = std::move(other._data);
      _t = std::move(other._t);
      return *this;
   }
    /////////////////////////////////////////////////////////////////////////////////////////
   ~AStar(){
      if (_t.joinable()){
         shutdown();
         _t.join();
      }
   }
   /////////////////////////////////////////////////////////////////////////////////////////
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