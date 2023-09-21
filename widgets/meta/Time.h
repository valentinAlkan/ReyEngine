#pragma once

#include <chrono>
#include <string>

class Time{
public:
   static Time& instance(){
      static Time instance;
      return instance;
   }
private:
   Time():_startTime(nowHiRes()){}
public:
   Time(Time const&)           = delete;
   void operator=(Time const&) = delete;
   static uint64_t getSecondsPassed();
   static std::chrono::time_point<std::chrono::high_resolution_clock> nowHiRes();
//   static std::string nowHiResString();
   static std::chrono::time_point<std::chrono::steady_clock> nowSteady();
//   static std::string nowSteadyString();
   static std::chrono::time_point<std::chrono::system_clock> nowSystem();
   static std::string nowSystemString();
   static uint64_t getCurrentFrameCount();
private:
   static std::string serializeTimePoint( const std::chrono::system_clock::time_point& time, const std::string& format);
   std::chrono::time_point<std::chrono::high_resolution_clock> _startTime; //the time since we started the application
   uint64_t _frame_count = 0;
   friend class Application;
};
