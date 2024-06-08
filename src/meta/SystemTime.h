#pragma once

#include <chrono>
#include <string>
#include <memory>
#include <vector>

namespace ReyEngine{
   class Timer;
   class SystemTime{
   public:
      static SystemTime& instance(){
         static SystemTime instance;
         return instance;
      }
   private:
      SystemTime(): _startTime(nowHiRes()){}
   public:
      SystemTime(SystemTime const&) = delete;
      void operator=(SystemTime const&) = delete;
      static uint64_t getSecondsPassed();
      static std::chrono::time_point<std::chrono::high_resolution_clock> nowHiRes();
   //   static std::string nowHiResString();
      static std::chrono::time_point<std::chrono::steady_clock> nowSteady();
   //   static std::string nowSteadyString();
      static std::chrono::time_point<std::chrono::system_clock> nowSystem();
      static std::string nowSystemString();
      static uint64_t getCurrentFrameCount();
      static std::shared_ptr<Timer> newTimer(std::chrono::milliseconds timeout);
   protected:
      static void processTimers();
   //   static void updateTimer(std::shared_ptr<Timer>);
   private:
      static std::string serializeTimePoint( const std::chrono::system_clock::time_point& time, const std::string& format);
      std::chrono::time_point<std::chrono::high_resolution_clock> _startTime; //the time since we started the application
      std::vector<std::weak_ptr<Timer>> _timers;
      uint64_t _frame_count = 0;
      friend class Application;
      friend class Timer;
      friend class Window;
   };
}