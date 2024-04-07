#pragma once
#include "DrawInterface.h"
#include "Component.h"
#include <thread>
#include <chrono>
#include "Application.h"

template <size_t dimensionCount, unsigned int dimensionSize>
class AStar : public Component {
public:
   AStar(const std::string& name = "Default")
   : Component(name)
   {
      _requestShutdown = std::make_unique<bool>();
      _data = std::make_unique<std::array<std::array<int, dimensionSize>, dimensionCount>>();
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
   void run(bool& requestShutdown, std::array<std::array<int, dimensionSize>, dimensionCount>& data) {
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
   std::unique_ptr<std::array<std::array<int, dimensionSize>, dimensionCount>> _data;
};