#pragma once
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <stdexcept>
#include "Event.h"

class BaseWidget;
using EventHandler = std::function<void(const BaseEvent&)>;
template <typename T>
using TypedEventHandler = std::function<void(const T&)>;
//using EventHandler = void(*)(const std::shared_ptr<Event>& Event);
using Publisher = std::shared_ptr<BaseWidget>;
using Subscriber = std::weak_ptr<BaseWidget>;
using EventName = std::string;
using EventCallbackMap = std::map<Subscriber,std::vector<EventHandler>, std::owner_less<>>;
class EventManager{
public:
   static EventManager& instance(){static EventManager instance; return instance;}
   template <typename T>
   static void publish(const Event<T>& event){
      auto& publisher = event.publisher;
      //find the publisher
      auto found = instance()._eventMap.find(publisher);
      if (found == instance()._eventMap.end()){
         //the publisher isn't registered at all
         return;
      }

      auto _ev = found->second.find(event.eventType);
      if (_ev == found->second.end()){
         //registered publisher doesn't have any events to publish
         return;
      }

      //get the vector of subscriber event maps
      auto& subscribers = _ev->second;
      for (auto it=subscribers.begin(); it!= subscribers.end(); /**/){
         auto& weakSubscriber = it->first;
         if (weakSubscriber.expired()){
            //subscriber is dead, long live the subscriber
            it = subscribers.erase(it);
         } else {
            //call every callback
            auto &handlers = it->second;
            for (auto &fx: handlers) {
               fx(event);
            }
            it++;
         }
      }
   }
   static void subscribe(Publisher&, const std::string& eventType, Subscriber, EventHandler);
   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber, EventHandler);
   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber);
   static void unsubscribe(Publisher& publisher, const std::string& eventType);
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










//test

class MultipleInheritableEnableSharedFromThis
: public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
public:
   virtual ~MultipleInheritableEnableSharedFromThis()
   {}
};

template <class T>
class inheritable_enable_shared_from_this
      : virtual public MultipleInheritableEnableSharedFromThis {
public:
   std::shared_ptr<T> shared_from_this() {
      return std::dynamic_pointer_cast<T>(
            MultipleInheritableEnableSharedFromThis::shared_from_this()
      );
   }
   /* Utility method to easily downcast.
    * Useful when a child doesn't inherit directly from enable_shared_from_this
    * but wants to use the feature.
    */
   template <class Down>
   std::shared_ptr<Down> downcasted_shared_from_this() {
      return std::dynamic_pointer_cast<Down>(
            MultipleInheritableEnableSharedFromThis::shared_from_this()
      );
   }
};




class EventPublisher;
struct TestEventBase{};
struct TestEvent : public TestEventBase {
   static constexpr char TESTEVENT[] = "TESTEVENT";
   TestEvent(const std::shared_ptr<EventPublisher>& publisher) {}
   std::string eventType = TESTEVENT;
};

class EventSubscriber : public inheritable_enable_shared_from_this<EventSubscriber>{
public:
   template <typename T>
   void subscribe(std::shared_ptr<EventPublisher> publisher, const std::string& eventType, std::function<void(const T&)> typedEventHandler){
      static_assert(std::is_base_of_v<TestEvent, T>);
      std::shared_ptr<EventSubscriber> me = shared_from_this();

      auto adapter = [typedEventHandler](const TestEventBase& baseEvent){
         auto s = (T&)baseEvent;
         typedEventHandler(s);
      };
      publisher->addSubscriber(me, eventType, adapter);
   };
//   template <typename T>
//   static void subscribe(Publisher&, const std::string& eventType, Subscriber, EventHandler){};
//   template <typename T>
//   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber, EventHandler){};
//   template <typename T>
//   void unsubscribe(Publisher& publisher, const std::string& eventType, EventHandler){};
};


using TestEventHandler = std::function<void(const TestEventBase&)>;
using TestEventCallbackMap = std::map<std::weak_ptr<EventSubscriber>,std::vector<TestEventHandler>, std::owner_less<>>;
class EventPublisher : public inheritable_enable_shared_from_this<EventPublisher>{
public:
   template <typename T>
   void publish(const T& event){
      static_assert(std::is_base_of_v<TestEventBase, T>); //compile time check
      auto publisher = shared_from_this();
      auto _ev = _eventMap.find(event.eventType);
      if (_ev == _eventMap.end()){
         //publisher doesn't have any events to publish
         return;
      }

      //get the vector of subscriber event maps
      auto& subscribers = _ev->second;
      for (auto it=subscribers.begin(); it!= subscribers.end(); /**/){
         auto& weakSubscriber = it->first;
         if (weakSubscriber.expired()){
            //subscriber is dead, long live the subscriber
            it = subscribers.erase(it);
         } else {
            //call every callback
            auto &handlers = it->second;
            for (auto& fx: handlers) {
               //make safe call w/ base
               fx((TestEventBase&)event);
            }
            it++;
         }
      }
   }
   void addSubscriber(std::weak_ptr<EventSubscriber> subscriber, const std::string& eventType, std::function<void(const TestEventBase&)> fx) {
      //assumed to be passing a valid subscriber pointer
      if (subscriber.expired()){
         throw std::runtime_error("Bad ptr!");
      }
      auto _ev = _eventMap.find(eventType);
      if (_ev == _eventMap.end()){
         //registered publisher doesn't have any events to publish
         _eventMap[eventType] = TestEventCallbackMap();
      }

      //get the vector of subscriber event maps
      auto& subscribers = _eventMap[eventType]; //call again since iterator could be invalid
      auto found = subscribers.find(subscriber);
      if (found == subscribers.end()){
         //create new set
         auto vec = _eventMap[eventType][subscriber] = std::vector<TestEventHandler>();
      }
      _eventMap[eventType][subscriber].push_back(fx);
   }

   std::map<EventName, TestEventCallbackMap> _eventMap;
};
