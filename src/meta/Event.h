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
static constexpr int getUniqueEventId(){return CLASSNAME##_UNIQUE_ID;}
#define EVENT_GET_NAME(CLASSNAME) static std::string getEventName() {return #CLASSNAME + std::string("_") + std::to_string(CLASSNAME##_UNIQUE_ID);}
#define EVENT_CTOR_SIMPLE(CLASSNAME, PARENTCLASS, ...) \
EVENT_GENERATE_UNIQUE_ID(CLASSNAME)               \
EVENT_GET_NAME(CLASSNAME)                                     \
explicit CLASSNAME(const std::shared_ptr<EventPublisher> publisher, ##__VA_ARGS__): PARENTCLASS(CLASSNAME##_UNIQUE_ID, publisher)
//NOTE: event publisher is a pointer copy and not a reference because toEventPublisher expects an lvalue, which toEventPublisher is not;


#define EVENT_CTOR_SIMPLE_OVERRIDABLE(CLASSNAME, PARENTCLASS) \
EVENT_GENERATE_UNIQUE_ID(CLASSNAME)                           \
EVENT_GET_NAME(CLASSNAME)                                     \
explicit CLASSNAME(EventId eventId, const std::shared_ptr<EventPublisher>& publisher): PARENTCLASS(eventId, publisher)

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class EventSubscriber;
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
   std::shared_ptr<EventSubscriber> subscriber;
};

template <typename T>
class Event : public BaseEvent {
public:
   Event(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
   : BaseEvent(eventId, publisher)
   {
//      std::cout << T::getEventName() << " has eventId " << T::getUniqueEventId() << std::endl;
//      std::cout << "Event has typeid " << getTy
   }
   template <typename Other>
   bool isEvent() const {
      static_assert(std::is_base_of_v<T, Other>);
      return eventId == Other::getUniqueEventId();
   }
   template <typename Other>
   const Other& toEventType() const {
      static_assert(std::is_base_of_v<T, Other>);
      return static_cast<const Other&>(*this);
   }
};
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class BaseWidget;
using EventHandler = std::function<void(const BaseEvent&)>;
using EventCallbackMap = std::map<std::weak_ptr<EventSubscriber>,std::vector<EventHandler>, std::owner_less<>>;
class EventPublisher : public inheritable_enable_shared_from_this<EventPublisher>{
public:
    void addSubscriber(std::weak_ptr<EventSubscriber> subscriber, std::string eventName, std::function<void(const BaseEvent&)> fx) {
        //assumed to be passing a valid subscriber pointer
        if (subscriber.expired()){
            throw std::runtime_error("Bad ptr!");
        }
        auto _ev = _eventMap.find(eventName);
        if (_ev == _eventMap.end()){
            //registered publisher doesn't have any events to publish
            _eventMap[eventName] = EventCallbackMap();
        }

        //get the vector of subscriber event maps
        auto& subscribers = _eventMap[eventName]; //call again since iterator could be invalid
        auto found = subscribers.find(subscriber);
        if (found == subscribers.end()){
            //create new set
            auto vec = _eventMap[eventName][subscriber];
        }
        _eventMap[eventName][subscriber].push_back(fx);
    }
    template <typename T>
    void publish(const T& event){
        static_assert(std::is_base_of_v<BaseEvent, T>); //compile time check
        auto publisher = downcasted_shared_from_this<EventPublisher>();
//      std::cout << "Publishing eventType " << T::getUniqueEventId() << "(" << T::getEventName() << ")" << std::endl;
        auto _ev = _eventMap.find(T::getEventName());
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
                //set the subscriber
                ((BaseEvent&)event).subscriber = weakSubscriber.lock();
                //call every callback WITH A MATCHING EVENT TYPE!
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
    std::shared_ptr<EventPublisher> toEventPublisher(){return downcasted_shared_from_this<EventPublisher>();}
private:
    std::map<std::string, EventCallbackMap> _eventMap;
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
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
      publisher->addSubscriber(me, T::getEventName(), adapter);
   };
   std::shared_ptr<EventSubscriber> toEventSubscriber(){return inheritable_enable_shared_from_this<EventSubscriber>::shared_from_this();}
   std::shared_ptr<BaseWidget> toBaseWidget();
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


struct TestEvent1 : public Event<TestEvent1> {
    EVENT_CTOR_SIMPLE(TestEvent1, Event<TestEvent1>){}
};

struct TestEvent2 : public Event<TestEvent2> {
    EVENT_CTOR_SIMPLE(TestEvent2, Event<TestEvent2>){}
};