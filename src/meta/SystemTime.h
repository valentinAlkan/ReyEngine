#pragma once

#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <thread>

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
  namespace Time{
     ///////////////////////////////////////////////////////////////
     //Ensures we run at the desired rate
     class RateLimiter{
     public:
        using us = std::chrono::microseconds;
        RateLimiter(double hz)
        : _targetHz(hz)
        , _targetPeriod((long long)(1.0 / hz * 1000000))
        {
           _timestamp = std::chrono::steady_clock::now();
        }
        void wait(){
           auto now = std::chrono::steady_clock::now();
           auto elapsed = now-_timestamp;
           auto timeToWait = _targetPeriod - elapsed;
           //slow down if necessary to achieve target loop rate
           if (elapsed < _targetPeriod){
              std::this_thread::sleep_until(now + timeToWait);
           }
           now = std::chrono::steady_clock::now();
           elapsed = now-_timestamp;
           _actualPeriod = std::chrono::microseconds(elapsed.count());
           _actualHz = 1.0 / std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() * 1000000.0;
           _timestamp = std::chrono::steady_clock::now();
        }
        us getTargetPeriod(){return _targetPeriod;}
        double getTargetHz(){return _targetHz;}
        us getActualPeriod(){return _actualPeriod;}
        double getActualHz(){return _actualHz;}
     private:
        const us _targetPeriod;
        const double _targetHz;
        double _actualHz;
        us _actualPeriod;
        std::chrono::steady_clock::time_point _timestamp;
     };
  }


}