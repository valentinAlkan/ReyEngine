#pragma once
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "SharedFromThis.h"
#include <iostream>
template<size_t N, size_t I=0>
struct hash_calc {
    static constexpr size_t apply (const char (&s)[N]) {
        return  (hash_calc<N, I+1>::apply(s) ^ s[I]) * 16777619u;
    };
};

template<size_t N>
struct hash_calc<N,N> {
    static constexpr size_t apply (const char (&s)[N]) {
        return  2166136261u;
    };
};

template<size_t N>
constexpr size_t hash ( const char (&s)[N] ) {
    return hash_calc<N>::apply(s);
}

// Helper macros for stringifying and concatenation
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS) \
    static constexpr long long unsigned int CLASSNAME##_UNIQUE_ID = hash(TOSTRING(CLASSNAME) TOSTRING(PARENTCLASS)); \
    static constexpr long long unsigned int getUniqueEventId() { return CLASSNAME##_UNIQUE_ID; }

#define EVENT_GET_NAME(CLASSNAME) \
    static std::string getEventName() { return #CLASSNAME + std::string("_") + std::to_string(CLASSNAME##_UNIQUE_ID); }

#define EVENT_CTOR_SIMPLE(CLASSNAME, PARENTCLASS, ...) \
    EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS) \
    explicit CLASSNAME(const std::shared_ptr<ReyEngine::EventPublisher> publisher, ##__VA_ARGS__) : PARENTCLASS(CLASSNAME::getUniqueEventId(), publisher)

//NOTE: event publisher is a pointer copy and not a reference because toEventPublisher expects an lvalue, which toEventPublisher is not;


#define EVENT_CTOR_SIMPLE_OVERRIDABLE(CLASSNAME, PARENTCLASS) \
EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS)              \
explicit CLASSNAME(ReyEngine::EventId eventId, const std::shared_ptr<ReyEngine::EventPublisher>& publisher): PARENTCLASS(eventId, publisher)

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
namespace ReyEngine{
   class EventSubscriber;
   using EventId = unsigned long long;
   class EventPublisher;
   class BaseEvent{
   public:
      BaseEvent(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
      : publisher(publisher)
      , eventId(eventId)
      {}
      BaseEvent(const BaseEvent& other): BaseEvent(other.eventId, other.publisher)
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
      {}
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

      template <typename Other>
      Other& toEventType() {
         static_assert(std::is_base_of_v<T, Other>);
         return static_cast<Other&>(*this);
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
       template <typename T>
       void addSubscriber(std::weak_ptr<EventSubscriber> subscriber, std::function<void(const BaseEvent&)> fx) {
           //assumed to be passing a valid subscriber pointer
           if (subscriber.expired()){
               throw std::runtime_error("Bad ptr!");
           }
           auto eventId = T::getUniqueEventId();
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
               _eventMap[eventId][subscriber];
           }
//           std::cout << "subscribing to event " << eventId << std::endl;
           _eventMap[eventId][subscriber].push_back(fx);
       }
       template <typename T>
       void publish(const T& event){
           static_assert(std::is_base_of_v<BaseEvent, T>); //compile time check
           auto publisher = downcasted_shared_from_this<EventPublisher>();
//           std::cout << "Publishing event " << T::getUniqueEventId() << std::endl;
           //publish by virtual eventId so we can publish from the base class
           auto _ev = _eventMap.find(event.eventId);
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

      template <typename T>
      std::shared_ptr<T> toPublisherType(){
         static_assert(std::is_base_of_v<EventPublisher, T>);
         return downcasted_shared_from_this<T>();
       }
   protected:
      std::shared_ptr<EventPublisher> toEventPublisher(){return downcasted_shared_from_this<EventPublisher>();}
   private:
       std::map<int, EventCallbackMap> _eventMap;
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
         publisher->addSubscriber<T>(me, adapter);
      };
      std::shared_ptr<EventSubscriber> toEventSubscriber(){return inheritable_enable_shared_from_this<EventSubscriber>::shared_from_this();}
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
}