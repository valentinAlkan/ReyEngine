#include "EventManager.h"
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::publish(Publisher& publisher, const std::shared_ptr<Event>& event) {
   //find the publisher
   auto found = instance()._eventMap.find(publisher);
   if (found == instance()._eventMap.end()){
      //the publisher isn't registered at all
      return;
   }

   auto _ev = found->second.find(event->eventType);
   if (_ev == found->second.end()){
      //registered publisher doesn't have any events to publish
      return;
   }

   //get the vector of subscriber event maps
   auto& subscribers = _ev->second;
   for (auto it=subscribers.begin(); it!= subscribers.end(); it++){
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

/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::subscribe(Publisher& publisher, std::string eventType, Subscriber subber, EventHandler fx) {
   //assumed to be passing a valid subscriber pointer
   auto foundpublisher = instance()._eventMap.find(publisher);
   if (foundpublisher == instance()._eventMap.end()){
      //the publisher isn't registered at all
      instance()._eventMap[publisher] = map<EventName, EventCallbackMap>();
   }

   auto _ev = foundpublisher->second.find(eventType);
   if (_ev == foundpublisher->second.end()){
      //registered publisher doesn't have any events to publish
      instance()._eventMap[publisher][eventType] = EventCallbackMap();
   }

   //get the vector of subscriber event maps
   auto& subscribers = _ev->second;
   for (auto it=subscribers.begin(); it!= subscribers.end(); ){
      auto& weakSubscriber = it->first;
      if (weakSubscriber.expired()){
         //subscriber is dead, long live the subscriber
         it = subscribers.erase(it);
         continue;
      } else {
         //call every callback
         // add the eventHandler to the set
         auto &vec = it->second;
         //does not check if already inserted - may call same callback twice if same subscriber subscribes multiple times
         vec.push_back(fx);
         ++it;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::unsubscribe(Publisher &publisher) {
   //assumed to be passing a valid subscriber pointer
   auto it = instance()._eventMap.find(publisher);
   if (it == instance()._eventMap.end()){
      //the publisher isn't registered at all
      return;
   }
   instance()._eventMap.erase(it);
}

/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::unsubscribe(Publisher &publisher, std::string eventType) {
   //assumed to be passing a valid subscriber pointer
   auto it = instance()._eventMap.find(publisher);
   if (it == instance()._eventMap.end()) {
      //the publisher isn't registered at all
      return;
   }
   auto &eventmap = it->second;
   auto eventit = eventmap.find(eventType);
   if (eventit == eventmap.end()) {
      //event type not registered
      return;
   }
   eventmap.erase(eventit);
}

/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::unsubscribe(Publisher &publisher, std::string eventType, Subscriber subscriber) {
   //assumed to be passing a valid subscriber pointer
   auto it = instance()._eventMap.find(publisher);
   if (it == instance()._eventMap.end()) {
      //the publisher isn't registered at all
      return;
   }
   auto& eventmap = it->second;
   auto eventit = eventmap.find(eventType);
   if (eventit == eventmap.end()) {
      //event type not registered
      return;
   }

   auto& subscriberMap = eventit->second;
   auto subscriberit = subscriberMap.find(subscriber);
   if (subscriberit == subscriberMap.end()) {
      // subscriber not subscribed
      return;
   }
   subscriberMap.erase(subscriberit);
}

/////////////////////////////////////////////////////////////////////////////////////////
void EventManager::unsubscribe(Publisher &publisher, std::string eventType, Subscriber subscriber, EventHandler handler) {
//assumed to be passing a valid subscriber pointer
   auto it = instance()._eventMap.find(publisher);
   if (it == instance()._eventMap.end()) {
      //the publisher isn't registered at all
      return;
   }
   auto& eventmap = it->second;
   auto eventit = eventmap.find(eventType);
   if (eventit == eventmap.end()) {
      //event type not registered
      return;
   }

   auto& subscriberMap = eventit->second;
   auto subscriberit = subscriberMap.find(subscriber);
   if (subscriberit == subscriberMap.end()) {
      // subscriber not subscribed
      return;
   }

   auto& handlers = subscriberit->second;
   auto handlerit = handlers.begin();
   for (auto handlerit = handlers.begin(); handlerit != handlers.end(); ){
      if (handlerit->target<EventHandler>() == handler.target<EventHandler>()){
         //these are the same function
         handlerit = handlers.erase(handlerit);
      } else {
         handlerit++;
      }
   }
}