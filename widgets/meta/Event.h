#pragma once
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "SharedFromThis.h"

#define EVENT_GENERATE_UNIQUE_ID(CLASSNAME) static constexpr int CLASSNAME##_UNIQUE_ID = __COUNTER__; \
static EventId getUniqueEventId(){return CLASSNAME##_UNIQUE_ID;}
#define EVENT_CTOR_SIMPLE(CLASSNAME, PARENTCLASS) \
EVENT_GENERATE_UNIQUE_ID(CLASSNAME)                                                  \
explicit CLASSNAME(const std::shared_ptr<EventPublisher>& publisher): PARENTCLASS(CLASSNAME##_UNIQUE_ID, publisher)

#define EVENT_CTOR_SIMPLE_OVERRIDABLE(CLASSNAME, PARENTCLASS) \
EVENT_GENERATE_UNIQUE_ID(CLASSNAME)                           \
explicit CLASSNAME(EventId eventId, const std::shared_ptr<EventPublisher>& publisher): PARENTCLASS(eventId, publisher)


using EventId = int;
class EventPublisher;
class BaseEvent{
public:
   BaseEvent(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
   : eventId(eventId)
   , publisher(publisher)
   {}
   const std::shared_ptr<EventPublisher>& publisher;
   const EventId eventId;
};

template <typename T>
class Event : public BaseEvent {
public:
   Event(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
   : BaseEvent(eventId, publisher)
   {}
   template <typename Other>
   bool isEvent(){
      static_assert(std::is_base_of_v<T, Other>);
      return eventId == Other::getUniqueEventId();
   }
   template <typename Other>
   const Other& toEventType(){
      static_assert(std::is_base_of_v<T, Other>);
      return static_cast<Other&>(*this);
   }
};

class EventPublisher;
class EventSubscriber : public inheritable_enable_shared_from_this<EventSubscriber>{
public:
   template <typename T>
   void subscribe(std::shared_ptr<EventPublisher> publisher, std::function<void(const T&)> typedEventHandler){
      static_assert(std::is_base_of_v<BaseEvent, T>);
      std::shared_ptr<EventSubscriber> me = shared_from_this();

      auto adapter = [typedEventHandler](const BaseEvent& baseEvent){
         auto s = (T&)baseEvent;
         typedEventHandler(s);
      };
      publisher->addSubscriber(me, T::getUniqueEventId(), adapter);
   };
//   template <typename T>
//   static void subscribe(Publisher&, const std::string& eventType, Subscriber, EventHandler){};
//   template <typename T>
//   static void unsubscribe(Publisher& publisher, const std::string& eventType, Subscriber, EventHandler){};
//   template <typename T>
//   void unsubscribe(Publisher& publisher, const std::string& eventType, EventHandler){};
};

using EventHandler = std::function<void(const BaseEvent&)>;
using EventCallbackMap = std::map<std::weak_ptr<EventSubscriber>,std::vector<EventHandler>, std::owner_less<>>;
class BaseWidget;
class EventPublisher : public inheritable_enable_shared_from_this<EventPublisher>{
public:
   void addSubscriber(std::weak_ptr<EventSubscriber> subscriber, EventId eventId, std::function<void(const BaseEvent&)> fx) {
      //assumed to be passing a valid subscriber pointer
      if (subscriber.expired()){
         throw std::runtime_error("Bad ptr!");
      }
      auto _ev = _eventMap.find(eventId);
      if (_ev == _eventMap.end()){
         //registered publisher doesn't have any events to publish
         _eventMap[eventId] = EventCallbackMap();
      }

      //get the vector of subscriber event maps
      auto& subscribers = _eventMap[eventId]; //call again since iterator could be invalid
      auto found = subscribers.find(subscriber);
      if (found == subscribers.end()){
         //create new set
         auto vec = _eventMap[eventId][subscriber];
      }
      _eventMap[eventId][subscriber].push_back(fx);
   }
   template <typename T>
   void publish(const T& event){
      static_assert(std::is_base_of_v<BaseEvent, T>); //compile time check
      auto publisher = shared_from_this();
      auto _ev = _eventMap.find(T::getUniqueEventId());
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
               fx((BaseEvent&)event);
            }
            it++;
         }
      }
   }
   std::shared_ptr<BaseWidget> toBaseWidget();
protected:
   std::shared_ptr<EventPublisher> toEventPublisher(){return inheritable_enable_shared_from_this<EventPublisher>::shared_from_this();}
private:
   std::map<EventId, EventCallbackMap> _eventMap;
};

struct TestEvent1 : public Event<TestEvent1> {
   EVENT_CTOR_SIMPLE(TestEvent1, Event<TestEvent1>){}
};

struct TestEvent2 : public Event<TestEvent2> {
   EVENT_CTOR_SIMPLE(TestEvent2, Event<TestEvent2>){}
};
