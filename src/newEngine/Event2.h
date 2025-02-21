#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "TypeTree.h"
//#include "SharedFromThis.h"
//template<size_t N, size_t I=0>
//struct hash_calc {
//   static constexpr size_t apply (const char (&s)[N]) {
//      return  (hash_calc<N, I+1>::apply(s) ^ s[I]) * 16777619u;
//   };
//};
//
//template<size_t N>
//struct hash_calc<N,N> {
//   static constexpr size_t apply (const char (&s)[N]) {
//      return  2166136261u;
//   };
//};
//
//template<size_t N>
//constexpr size_t hash ( const char (&s)[N] ) {
//   return hash_calc<N>::apply(s);
//}
//
//// Helper macros for stringifying and concatenation
//#define STRINGIFY(x) #x
//#define TOSTRING(x) STRINGIFY(x)
//
//#define EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS) \
//    static constexpr long long unsigned int CLASSNAME##_UNIQUE_ID = hash(TOSTRING(CLASSNAME) TOSTRING(PARENTCLASS)); \
//    static constexpr long long unsigned int getUniqueEventId() { return CLASSNAME##_UNIQUE_ID; }
//
//#define EVENT_GET_NAME(CLASSNAME) \
//    static std::string getEventName() { return #CLASSNAME + std::string("_") + std::to_string(CLASSNAME##_UNIQUE_ID); }
//
//#define EVENT_CTOR_SIMPLE(CLASSNAME, PARENTCLASS, ...) \
//    EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS) \
//    explicit CLASSNAME(const std::shared_ptr<ReyEngine::EventPublisher> publisher, ##__VA_ARGS__) : PARENTCLASS(CLASSNAME::getUniqueEventId(), publisher)
//
////NOTE: event publisher is a pointer copy and not a reference because toEventPublisher expects an lvalue, which toEventPublisher is not;
//
//
//#define EVENT_CTOR_SIMPLE_OVERRIDABLE(CLASSNAME, PARENTCLASS) \
//EVENT_GENERATE_UNIQUE_ID(CLASSNAME, PARENTCLASS)              \
//explicit CLASSNAME(ReyEngine::EventId eventId, const std::shared_ptr<ReyEngine::EventPublisher>& publisher): PARENTCLASS(eventId, publisher)

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//namespace ReyEngine{
//   class EventSubscriber;
//   using EventId = unsigned long long;
//   class EventPublisher;
//   class BaseEvent{
//   public:
//      BaseEvent(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
//            : publisher(publisher)
//            , eventId(eventId)
//      {}
//      BaseEvent(const BaseEvent& other)
//            : BaseEvent(other.eventId, other.publisher)
//      {
//         subscriber = other.subscriber;
//      }
//      const std::shared_ptr<EventPublisher>& publisher;
//      const EventId eventId;
//      std::shared_ptr<EventSubscriber> subscriber;
//   };
//
//   template <typename T>
//   class Event : public BaseEvent {
//   public:
//      Event(EventId eventId, const std::shared_ptr<EventPublisher>& publisher)
//      : BaseEvent(eventId, publisher)
//      {}
//      Event(const Event& other)
//      : BaseEvent(other)
//      {}
//      template <typename Other>
//      [[nodiscard]] bool isEvent() const {
//         static_assert(std::is_base_of_v<T, Other>);
//         return eventId == Other::getUniqueEventId();
//      }
//      template <typename Other>
//      const Other& toEventType() const {
//         static_assert(std::is_base_of_v<T, Other>);
//         return static_cast<const Other&>(*this);
//      }
//
//      template <typename Other>
//      Other& toEventType() {
//         static_assert(std::is_base_of_v<T, Other>);
//         return static_cast<Other&>(*this);
//      }
//   };
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//   using EventHandler = std::function<void(const BaseEvent&)>;
//   using EventCallbackMap = std::map<std::weak_ptr<EventSubscriber>,std::vector<EventHandler>, std::owner_less<>>;
//   class EventPublisher : public std::enable_shared_from_this<EventPublisher>{
//   public:
//      template <typename T>
//      void addSubscriber(std::weak_ptr<EventSubscriber> subscriber, std::function<void(const BaseEvent&)> fx) {
//         //assumed to be passing a valid subscriber pointer
//         if (subscriber.expired()){
//            throw std::runtime_error("Bad ptr!");
//         }
//         auto eventId = T::getUniqueEventId();
//         auto _ev = _eventMap.find(eventId);
//         if (_ev == _eventMap.end()){
//            //registered publisher doesn't have any events to publish
//            _eventMap[eventId] = EventCallbackMap();
//         }
//
//         //get the vector of subscriber event maps
//         auto& subscribers = _eventMap[eventId]; //call again since iterator could be invalid
//         auto found = subscribers.find(subscriber);
//         if (found == subscribers.end()){
//            //create new set
//            _eventMap[eventId][subscriber];
//         }
////           std::cout << "subscribing to event " << eventId << std::endl;
//         _eventMap[eventId][subscriber].push_back(fx);
//      }
//      template <typename T>
//      void publishMutable(T& event){
//         publish(static_cast<const T&>(event));
//      }
//      template <typename T>
//      void publish(const T& event){
//         static_assert(std::is_base_of_v<BaseEvent, T>); //compile time check
////         auto publisher = shared_from_this();
////           std::cout << "Publishing event " << T::getUniqueEventId() << std::endl;
//         //publish by virtual eventId so we can publish from the base class
//         auto _ev = _eventMap.find(event.eventId);
//         if (_ev == _eventMap.end()){
//            //publisher doesn't have any events to publish
//            return;
//         }
//
//         //get the vector of subscriber event maps
//         auto& subscribers = _ev->second;
//         for (auto it=subscribers.begin(); it!= subscribers.end(); /**/){
//            auto& weakSubscriber = it->first;
//            if (weakSubscriber.expired()){
//               //subscriber is dead, long live the subscriber
//               it = subscribers.erase(it);
//            } else {
//               //set the subscriber
//               ((BaseEvent&)event).subscriber = weakSubscriber.lock();
//               //call every callback WITH A MATCHING EVENT TYPE!
//               auto &handlers = it->second;
//               for (auto& fx: handlers) {
//                  //make safe call w/ base
//                  fx((BaseEvent&)event);
//               }
//               it++;
//            }
//         }
//      }
//
//      template <typename T>
//      std::shared_ptr<T> toPublisherType(){
//         static_assert(std::is_base_of_v<EventPublisher, T>);
//         return shared_from_this();
//      }
//   protected:
////      std::shared_ptr<EventPublisher> toEventPublisher(){return downcasted_shared_from_this<EventPublisher>();}
//   private:
//      std::map<int, EventCallbackMap> _eventMap;
//   };
//
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//class EventSubscriber : public std::enable_shared_from_this<EventSubscriber>{
//   public:
//      template <typename T>
//      void subscribeMutable(std::shared_ptr<EventPublisher> publisher, std::function<void(T&)> typedEventHandler){
//         std::shared_ptr<EventSubscriber> me = shared_from_this();
//         auto adapter = [typedEventHandler](const BaseEvent& baseEvent) {
//            auto& s = const_cast<T&>(static_cast<const T&>(baseEvent));
//            typedEventHandler(s);
//         };
//         publisher->addSubscriber<T>(me, adapter);
//      }
//      template <typename T>
//      void subscribe(std::shared_ptr<EventPublisher> publisher, std::function<void(const T&)> typedEventHandler){
//         static_assert(std::is_base_of_v<BaseEvent, T>);
//         std::shared_ptr<EventSubscriber> me = shared_from_this();
//
//         auto adapter = [typedEventHandler](const BaseEvent& baseEvent){
//            auto s = (T&)baseEvent;
//            typedEventHandler(s);
//         };
//         publisher->addSubscriber<T>(me, adapter);
//      };
//      std::shared_ptr<EventSubscriber> toEventSubscriber(){return std::enable_shared_from_this<EventSubscriber>::shared_from_this();}
//   };
//
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//   /////////////////////////////////////////////////////////////////////////////////////////
//}

namespace ReyEngine{
   struct EventPublisher;
   struct EventSubscriber;
   struct BaseEvent {
      BaseEvent(EventPublisher* publisher, std::type_index eventId)
      : publisher(publisher)
      , eventId(eventId)
      {}
      [[nodiscard]] virtual std::type_index getEventId() const = 0;
      EventPublisher* publisher;
      std::type_index eventId;
   };

   template <typename T>
   struct Event : public BaseEvent {
   Event(EventPublisher* publisher): BaseEvent(publisher, typeid(T)){}
   [[nodiscard]] std::type_index getEventId() const override {return std::type_index(typeid(T));}
   };

   struct EventCallback {
      EventCallback(EventPublisher* publisher, EventSubscriber* subscriber, std::function<void(const BaseEvent&)> callback)
      : publisher(publisher)
      , subscriber(subscriber)
      , callback(callback)
      {}
      EventPublisher* publisher;
      EventSubscriber* subscriber;
      std::function<void(const BaseEvent&)> callback;
   };

   using EventHandler = std::function<void(const BaseEvent&)>;
   using EventCallbackMap = std::map<EventSubscriber*,std::vector<EventHandler>>;
   struct EventPublisher {
      virtual ~EventPublisher();
      std::map<std::type_index, EventCallbackMap> _eventMap;
   protected:
      template <typename T>
      void addSubscriber(EventSubscriber* subscriber, std::function<void(const BaseEvent&)> fx) {
         auto eventId = std::type_index(typeid(T));
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
            auto eventdescr = _eventMap[eventId][subscriber];
         }
   //           std::cout << "subscribing to event " << eventId << std::endl;
         _eventMap[eventId][subscriber].push_back(fx);
      }
//
      void removeSubscriptions(EventSubscriber* cancellingSubscriber) {
         for (auto itouter = _eventMap.begin(); itouter!=_eventMap.end();/**/){
            auto& callbackmap = itouter->second;
            for (auto itinner=callbackmap.begin(); itinner != callbackmap.end(); ++itinner){
               auto& subscriber = itinner->first;
               if (subscriber == cancellingSubscriber) {
                  //one map per subscriber, so we're done here
                  callbackmap.erase(subscriber);
                  break;
               }
            }
            if (callbackmap.empty()){
               //could have multiple event types per subscriber
               itouter = _eventMap.erase(itouter);
            } else {
               ++itouter;
            }
         }
      }
      template <typename T>
      void publishMutable(T& event){
         publish(static_cast<const T&>(event));
      }
   public:
      template <typename T>
      void publish(const T& event){
         static_assert(std::is_base_of_v<BaseEvent, T>); //compile time check
   //      auto publisher = downcasted_shared_from_this<EventPublisher>();
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
            //set the subscriber
            //call every callback WITH A MATCHING EVENT TYPE!
            auto &handlers = it->second;
            for (auto& fx: handlers) {
               //make safe call w/ base
               fx((BaseEvent&)event);
               it++;
            }
         }
      }
      friend class EventSubscriber;
   };

   class EventSubscriber {
   public:
      ~EventSubscriber(){
         //unsubscribe from all events
         while(!_subscriptions.empty()){
            cancelSubscription(_subscriptions.begin()->first);
         }
      }
//      template <typename T>
//      void subscribeMutable(EventPublisher* publisher, std::function<void(T&)> typedEventHandler){
//         std::shared_ptr<EventSubscriber> me = std::shared_from_this();
//         auto adapter = [typedEventHandler](const BaseEvent& baseEvent) {
//            auto& s = const_cast<T&>(static_cast<const T&>(baseEvent));
//            typedEventHandler(s);
//         };
//         publisher->addSubscriber<T>(me, adapter);
//      }
      template <typename T>
      void subscribe(EventPublisher* publisher, std::function<void(const T&)> typedEventHandler){
         static_assert(std::is_base_of_v<BaseEvent, T>);
         auto adapter = [typedEventHandler](const BaseEvent& baseEvent){
            auto s = (T&)baseEvent;
            typedEventHandler(s);
         };
         std::type_index tid = typeid(T);
         auto it = _subscriptions.try_emplace(publisher);
         it.first->second.push_back(tid);

         publisher->addSubscriber<T>(this, adapter);
      };
      //user cancel.
      void cancelSubscription(EventPublisher* deadPublisher){
         cancelSubscription(deadPublisher, false);
      }
   protected:
         //publisher doesn't want you as a customer anymore. doesn't reach back out to publisher.
      void cancelSubscription(EventPublisher* deadPublisher, bool fromPublisher){
         for (auto it=_subscriptions.begin(); it!=_subscriptions.end(); it++){
            auto publisher = it->first;
            if (publisher == deadPublisher){
               //found the dying publisher, kill its subscriptions
               if (!fromPublisher){
                  publisher->removeSubscriptions(this);
               }
               it = _subscriptions.erase(it);
               break;
            }
         }
      }
      EventSubscriber* toEventSubscriber(){return static_cast<EventSubscriber*>(this);}
      std::map<EventPublisher*, std::vector<std::type_index>> _subscriptions;
      friend class EventPublisher;
   };
}