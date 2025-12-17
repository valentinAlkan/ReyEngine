#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "TypeTree.h"
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

//not necessarily used but keeping here for later
template <typename T>
constexpr auto type_name() {
#if defined(__GNUG__) || defined(__clang__)
   std::string_view name = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
   std::string_view name = __FUNCSIG__;
    #else
        #error "Unsupported compiler"
#endif

   // Stripping logic varies by compiler
#if defined(__clang__)
   constexpr auto prefix = std::string_view{"auto type_name() [T = "};
        constexpr auto suffix = std::string_view{"]"};
#elif defined(__GNUG__)
   constexpr auto prefix = std::string_view{"constexpr auto type_name() [with T = "};
   constexpr auto suffix = std::string_view{"]"};
#elif defined(_MSC_VER)
   constexpr auto prefix = std::string_view{"auto __cdecl type_name<"};
        constexpr auto suffix = std::string_view{">(void)"};
#endif

   name.remove_prefix(prefix.size());
   name.remove_suffix(suffix.size());
   return name;
}

namespace ReyEngine{
   enum class PublishType {DO_PUBLISH, NO_PUBLISH};
   using EventId = size_t;
   struct EventPublisher;
   struct EventSubscriber;

   struct BaseEvent {
      BaseEvent(const EventPublisher* publisher, EventId eventId)
      : publisher(publisher)
      , eventId(eventId)
      {}
      virtual ~BaseEvent(){};
      template <typename T>
      T& toEventType() {
         static_assert(std::is_base_of_v<BaseEvent, T>);
         return static_cast<T&>(*this);
      }
      template <typename T>
      std::optional<const T*> isEvent() const {
         static_assert(std::is_base_of_v<BaseEvent, T>);
         if (eventId == T::ID){
            return dynamic_cast<const T*>(this);
         }
         return {};
      }
      const EventPublisher* publisher;
      const EventId eventId;
   };

   template <typename T, size_t EVENT_ID>
   struct Event : public BaseEvent {
   Event(const EventPublisher* publisher): BaseEvent(publisher, EVENT_ID){}
      template <typename Other>
      const Other& toEventType() const {
         static_assert(std::is_base_of_v<BaseEvent, Other>);
         return static_cast<const Other&>(*this);
      }

      template <typename Other>
      Other& toEventType() {
         static_assert(std::is_base_of_v<T, Other>);
         return static_cast<Other&>(*this);
      }
      //simpler than a case statement if you only care about one type
      bool isEvent(){return eventId == T::ID;}
   };

   struct EventCallback {
      EventCallback(EventPublisher& publisher, EventSubscriber& subscriber, std::function<void(const BaseEvent&)>& callback)
      : publisher(publisher)
      , subscriber(subscriber)
      , callback(callback)
      {}
      EventPublisher& publisher;
      EventSubscriber& subscriber;
      std::function<void(const BaseEvent&)> callback;
   };

   using EventHandler = std::function<void(const BaseEvent&)>;
   using EventCallbackMap = std::map<EventSubscriber*,std::vector<EventHandler>>;
   struct EventPublisher {
      virtual ~EventPublisher();
      std::map<EventId, EventCallbackMap> _eventMap;
      template <typename T>
      std::optional<const T*> as() const {
         auto thiz = dynamic_cast<const T*>(this);
         if (thiz){
            return thiz;
         }
         return {};
      }
      template <typename T>
      std::optional<T*> as() {
         auto thiz = dynamic_cast<T*>(this);
         if (thiz){
            return thiz;
         }
         return {};
      }
   protected:
      template <typename T>
      requires (std::is_base_of_v<BaseEvent, T>)
      void addSubscriber(EventSubscriber* subscriber, std::function<void(const BaseEvent&)> fx) {
         auto eventId = T::ID;
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
      requires (std::is_base_of_v<BaseEvent, T>)
      void addSubscriber(EventSubscriber* subscriber, std::function<void(BaseEvent&)>& fx) {
         auto constAdapter = [fx](const BaseEvent& event){ fx(const_cast<BaseEvent&>(event));};
         addSubscriber<T>(subscriber, constAdapter);
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
      template <typename T>
      void subscribeMutable(const std::shared_ptr<EventPublisher>& publisher, std::function<void(T&)> typedEventHandler){subscribeMutable(publisher.get(), typedEventHandler);}
      template <typename T>
      void subscribeMutable(EventPublisher* publisher, std::function<void(T&)> typedEventHandler){
         auto adapter = [typedEventHandler](const BaseEvent& baseEvent) {
            auto& s = const_cast<T&>(static_cast<const T&>(baseEvent));
            typedEventHandler(s);
         };
         publisher->addSubscriber<T>(this, adapter);
      }
      template <typename T>
      void subscribe(const std::shared_ptr<EventPublisher>& publisher, std::function<void(const T&)> typedEventHandler){subscribe(publisher.get(), typedEventHandler);}
      template <typename T>
      void subscribe(EventPublisher* publisher, std::function<void(const T&)> typedEventHandler){
         static_assert(std::is_base_of_v<BaseEvent, T>);
         auto adapter = [typedEventHandler](const BaseEvent& baseEvent){
            auto s = (T&)baseEvent;
            typedEventHandler(s);
         };
         std::type_index tid = typeid(T);
         if (!publisher) throw std::runtime_error("Cannot subscribe to null publisher!");
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

#define CONSTEXPR_EVENTID(EVENT_ID) static constexpr ReyEngine::EventId ID = EVENT_ID; \
                                    static constexpr ReyEngine::EventId getUniqueEventId(){return ID;} //temporary <- eases porting
#define EVENT(EVENT_NAME, UNIQUE_EVENT_ID) \
   struct EVENT_NAME : public ReyEngine::Event<EVENT_NAME, UNIQUE_EVENT_ID> { \
      CONSTEXPR_EVENTID(UNIQUE_EVENT_ID)                   \
      explicit EVENT_NAME(const ReyEngine::EventPublisher* publisher) : ReyEngine::Event<EVENT_NAME, UNIQUE_EVENT_ID>(publisher)


#define EVENT_ARGS(EVENT_NAME, UNIQUE_EVENT_ID, ...) \
   struct EVENT_NAME : public ReyEngine::Event<EVENT_NAME, UNIQUE_EVENT_ID> { \
      CONSTEXPR_EVENTID(UNIQUE_EVENT_ID)             \
      explicit EVENT_NAME(const ReyEngine::EventPublisher* publisher, __VA_ARGS__) : Event<EVENT_NAME, UNIQUE_EVENT_ID>(publisher)
