#pragma once
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include "Event.h"

class BaseWidget;
using EventHandler = std::function<void(const std::shared_ptr<Event>)>;
//using EventHandler = void(*)(const std::shared_ptr<Event>& Event);
using Publisher = std::shared_ptr<BaseWidget>;
using Subscriber = std::weak_ptr<BaseWidget>;
using EventName = std::string;
//using EventCallbackMap = std::map<Subscriber,std::unordered_set<EventHandler>, std::owner_less<>>;
using EventCallbackMap = std::map<Subscriber,std::vector<EventHandler>, std::owner_less<>>;
class EventManager{
public:
   static EventManager& instance(){static EventManager instance; return instance;}
   static void publish(Publisher&, const std::shared_ptr<Event>&);
   static void subscribe(Publisher&, std::string eventType, Subscriber, EventHandler);
   static void unsubscribe(Publisher& publisher, std::string eventType, Subscriber, EventHandler);
   static void unsubscribe(Publisher& publisher, std::string eventType, Subscriber);
   static void unsubscribe(Publisher& publisher, std::string eventType);
   static void unsubscribe(Publisher& publisher);
private:
   EventManager() {}
   std::map<Publisher,
         std::map<EventName,
               EventCallbackMap>> _eventMap;
public:
   EventManager(EventManager const&) = delete;
   void operator=(EventManager const&) = delete;
};