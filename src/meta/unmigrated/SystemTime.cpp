#include "SystemTime.h"
#include <sstream>
#include <iomanip>
#include "Timer.hpp"

using namespace std;
using namespace chrono;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::high_resolution_clock> SystemTime::nowHiRes() {
   return high_resolution_clock::now();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::steady_clock> SystemTime::nowSteady() {
   return steady_clock::now();
}
/////////////////////////////////////////////////////////////////////////////////////////
std::chrono::time_point<std::chrono::system_clock> SystemTime::nowSystem() {
   return system_clock::now();
}

/////////////////////////////////////////////////////////////////////////////////////////
uint64_t SystemTime::getCurrentFrameCount() {
   return SystemTime::instance()._frame_count;
};

/////////////////////////////////////////////////////////////////////////////////////////
uint64_t SystemTime::getSecondsPassed() {
   auto delta = nowHiRes() - SystemTime::instance()._startTime;
   return duration_cast<seconds>(delta).count();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string SystemTime::nowSystemString() {
   return serializeTimePoint(nowSystem(), "UTC: %Y-%m-%d %H:%M:%S");
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string SystemTime::serializeTimePoint(const std::chrono::system_clock::time_point &time, const std::string &format) {
   std::time_t tt = std::chrono::system_clock::to_time_t(time);
   std::tm tm = *std::gmtime(&tt); //GMT (UTC)
   //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
   std::stringstream ss;
   ss << std::put_time( &tm, format.c_str() );
   return ss.str();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Timer> SystemTime::newTimer(std::chrono::milliseconds timeout) {
   auto newTimer = std::shared_ptr<Timer>(new Timer(timeout));
   instance()._timers.push_back(newTimer);
   return newTimer;
}

/////////////////////////////////////////////////////////////////////////////////////////
void SystemTime::processTimers() {
   auto& timers = instance()._timers;
   for (auto it = timers.begin(); it!=timers.end();/**/){
      auto ptr = *it;
      if (ptr.expired()){
         //delete expired pointers
         it = timers.erase(it);
      } else {
         ptr.lock()->process(nowSteady());
         it++;
      }
   }
}