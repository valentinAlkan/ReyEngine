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
      _t = std::thread(&AStar::run, this, std::ref(_requestShutdown));
   }
   AStar(AStar&& other)
   : Component(other._name){
      _t = std::move(other._t);
      _requestShutdown = std::move(other._requestShutdown);
   }
   AStar& operator=(const AStar& other){
//   todo: finish - actually copy stuff
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
   void run(std::unique_ptr<bool>& requestShutdown){
      using namespace std::chrono;
      std::cout << "AStar::run::start" << std::endl;
      while (!requestShutdown){
         static int i=0;
         std::this_thread::sleep_for(1000ms);
         std::cout << "AStar::run::" << i++ << std::endl;
      }
      std::cout << "AStar::run::end " << std::endl;
   }
   std::thread _t;
   std::unique_ptr<bool> _requestShutdown;
   std::array<std::array<int, dimensionSize>, dimensionCount> _data;
};