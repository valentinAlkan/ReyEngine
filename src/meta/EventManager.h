#pragma once
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <stdexcept>
#include "SharedFromThis.h"
#include "Event.h"

//class BaseWidget;
//using EventHandler = std::function<void(const BaseEvent&)>;
//template <typename T>
//using TypedEventHandler = std::function<void(const T&)>;
////using EventHandler = void(*)(const std::shared_ptr<Event>& Event);
//using Publisher = std::shared_ptr<BaseWidget>;
//using Subscriber = std::weak_ptr<BaseWidget>;
//using EventCallbackMap = std::map<Subscriber,std::vector<EventHandler>, std::owner_less<>>;
//class EventManager{
//public:
//   static EventManager& instance(){static EventManager instance; return instance;}
//   template <typename T>
//   static void publish(const Event<T>& event){
//      auto& publisher = event.publisher;
//      //find the publisher
//      auto found = instance()._eventMap.find(publisher);
//      if (found == instance()._eventMap.end()){
//         //the publisher isn't registered at all
//         return;
//      }
//
//      auto _ev = found->second.find(event.eventType);
//      if (_ev == found->second.end()){
//         //registered publisher doesn't have any events to publish
//         return;
//      }
//
//      //get the vector of subscriber event maps
//      auto& subscribers = _ev->second;
//      for (auto it=subscribers.begin(); it!= subscribers.end(); /**/){
//         auto& weakSubscriber = it->first;
//         if (weakSubscriber.expired()){
//            //subscriber is dead, long live the subscriber
//            it = subscribers.erase(it);
//         } else {
//            //call every callback
//            auto &handlers = it->second;
//            for (auto &fx: handlers) {
//               fx(event);
//            }
//            it++;
//         }
//      }
//   }
//   static void subscribe(Publisher&, const std::string& eventType, Subscriber, EventHandler);
//   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber, EventHandler);
//   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber);
//   static void unsubscribe(Publisher& publisher, const std::string& eventType);
//   static void unsubscribe(Publisher& publisher);
//private:
//   EventManager() {}
//   std::map<Publisher,
//         std::map<EventName,
//               EventCallbackMap>> _eventMap;
//public:
//   EventManager(EventManager const&) = delete;
//   void operator=(EventManager const&) = delete;
//};
//
