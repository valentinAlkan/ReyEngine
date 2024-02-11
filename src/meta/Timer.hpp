#pragma once
#include "Event.h"
#include <chrono>
#include <string>
#include "SystemTime.h"
#include "Event.h"
#include "Property.h"

class Timer : public EventPublisher {
public:
   class TimeoutEvent : public Event<TimeoutEvent>{
   public:
      EVENT_CTOR_SIMPLE(TimeoutEvent, Event<TimeoutEvent>){}
   };

   void setAutoRestart(bool value){autoRestart = value;}
   void start(){running = true; timedOut = false;}
   void stop(){running = false;}
   void resetAndStop(){
      reset();
      stop();
   }
   void resetAndStart(){
      reset();
      start();
   }
   auto timeLeft(std::chrono::time_point<std::chrono::steady_clock> now){return now - (startTime + interval);}
   void setInterval(std::chrono::milliseconds ms){interval = ms;}
   std::chrono::milliseconds getInterval(){return interval;}
   bool isRunning(){return running;}
protected:
   Timer(std::chrono::milliseconds interval): interval(interval){}
   bool process(std::chrono::time_point<std::chrono::steady_clock> currentTime){
      if (currentTime > startTime + interval){
         //fire our timeout event
         publish(TimeoutEvent(toEventPublisher()));
         if (autoRestart){
            resetAndStart();
            return true;
         } else {
            resetAndStop();
            timedOut = true;
            return true;
         }
      }
      return false;
   }
private:
   void reset(){startTime = SystemTime::instance().nowSteady();}
   bool autoRestart = true;
   bool timedOut = false;
   std::chrono::milliseconds interval;
   std::chrono::time_point<std::chrono::steady_clock> startTime;
   bool running = false;
   friend class SystemTime;
};

struct TimerProperty : public Property<std::shared_ptr<Timer>>{
   TimerProperty(const std::string& instanceName, std::shared_ptr<Timer> defaultvalue)
   : Property<std::shared_ptr<Timer>>(instanceName, PropertyTypes::Timer, defaultvalue)
   {}
   std::string toString() const override {return std::to_string(value->getInterval().count());}
   std::shared_ptr<Timer> fromString(const std::string& str) {return SystemTime::newTimer(std::chrono::milliseconds(std::stoi(str)));}
};
