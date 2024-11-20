#pragma once

#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <stdexcept>

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
     class RateLimiter {
     public:
        using us = std::chrono::microseconds;
        explicit RateLimiter(double hz)
              : _targetHz(hz)
              , _targetPeriod(std::chrono::microseconds(static_cast<long long>(1000000.0 / hz)))
        {
           _timestamp = std::chrono::steady_clock::now();
        }

        void wait() {
           // Calculate how long we need to wait to hit our target period
           while (true) {
              auto now = std::chrono::steady_clock::now();
              auto elapsed = std::chrono::duration_cast<us>(now - _timestamp);

              if (elapsed >= _targetPeriod) {
                 break;
              }

              auto remaining = _targetPeriod - elapsed;
              if (remaining > us(1000)) {  // If more than 1ms remaining
                 std::this_thread::sleep_for(remaining - us(500));  // Sleep for slightly less than needed
              } else {
                 std::this_thread::yield();  // Busy-wait for fine-grained control
              }
           }

           // Update measurements
           auto now = std::chrono::steady_clock::now();
           _actualPeriod = std::chrono::duration_cast<us>(now - _timestamp);
           if (_actualPeriod.count() > 0) {
              _actualHz = 1000000.0 / static_cast<double>(_actualPeriod.count());
           }

           _timestamp = now;
        }

        us getTargetPeriod() const { return _targetPeriod; }
        double getTargetHz() const { return _targetHz; }
        us getActualPeriod() const { return _actualPeriod; }
        double getActualHz() const { return _actualHz; }

     private:
        const double _targetHz;
        const us _targetPeriod;
        std::chrono::steady_clock::time_point _timestamp;
        us _actualPeriod{};
        double _actualHz{};
     };
  }


}