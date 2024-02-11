//#include "EventManager.h"
//using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////
//void EventManager::subscribe(Publisher& publisher, const std::string& eventType, Subscriber subber, EventHandler fx) {
//   //assumed to be passing a valid subscriber pointer
//   auto foundpublisher = instance()._eventMap.find(publisher);
//   if (foundpublisher == instance()._eventMap.end()){
//      //the publisher isn't registered at all
//      instance()._eventMap[publisher] = map<EventName, EventCallbackMap>();
//   }
//
//   auto _ev = instance()._eventMap[publisher].find(eventType);
//   if (_ev == instance()._eventMap[publisher].end()){
//      //registered publisher doesn't have any events to publish
//      instance()._eventMap[publisher][eventType] = EventCallbackMap();
//   }
//
//   //get the vector of subscriber event maps
//   auto& subscribers = instance()._eventMap[publisher][eventType];
//   auto found = subscribers.find(subber);
//   if (found == subscribers.end()){
//      //create new set
//      instance()._eventMap[publisher][eventType][subber];
//   }
//   instance()._eventMap[publisher][eventType][subber].push_back(fx);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void EventManager::unsubscribe(Publisher &publisher) {
//   //assumed to be passing a valid subscriber pointer
//   auto it = instance()._eventMap.find(publisher);
//   if (it == instance()._eventMap.end()){
//      //the publisher isn't registered at all
//      return;
//   }
//   instance()._eventMap.erase(it);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void EventManager::unsubscribe(Publisher &publisher, const std::string& eventType) {
//   //assumed to be passing a valid subscriber pointer
//   auto it = instance()._eventMap.find(publisher);
//   if (it == instance()._eventMap.end()) {
//      //the publisher isn't registered at all
//      return;
//   }
//   auto &eventmap = it->second;
//   auto eventit = eventmap.find(eventType);
//   if (eventit == eventmap.end()) {
//      //event type not registered
//      return;
//   }
//   eventmap.erase(eventit);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void EventManager::unsubscribe(Publisher &publisher, const std::string& eventType, Subscriber subscriber) {
//   //assumed to be passing a valid subscriber pointer
//   auto it = instance()._eventMap.find(publisher);
//   if (it == instance()._eventMap.end()) {
//      //the publisher isn't registered at all
//      return;
//   }
//   auto& eventmap = it->second;
//   auto eventit = eventmap.find(eventType);
//   if (eventit == eventmap.end()) {
//      //event type not registered
//      return;
//   }
//
//   auto& subscriberMap = eventit->second;
//   auto subscriberit = subscriberMap.find(subscriber);
//   if (subscriberit == subscriberMap.end()) {
//      // subscriber not subscribed
//      return;
//   }
//   subscriberMap.erase(subscriberit);
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void EventManager::unsubscribe(Publisher &publisher, const std::string& eventType, Subscriber subscriber, EventHandler handler) {
////assumed to be passing a valid subscriber pointer
//   auto it = instance()._eventMap.find(publisher);
//   if (it == instance()._eventMap.end()) {
//      //the publisher isn't registered at all
//      return;
//   }
//   auto& eventmap = it->second;
//   auto eventit = eventmap.find(eventType);
//   if (eventit == eventmap.end()) {
//      //event type not registered
//      return;
//   }
//
//   auto& subscriberMap = eventit->second;
//   auto subscriberit = subscriberMap.find(subscriber);
//   if (subscriberit == subscriberMap.end()) {
//      // subscriber not subscribed
//      return;
//   }
//
//   auto& handlers = subscriberit->second;
//   for (auto handlerit = handlers.begin(); handlerit != handlers.end(); ){
//      if (handlerit->target<EventHandler>() == handler.target<EventHandler>()){
//         //these are the same function
//         handlerit = handlers.erase(handlerit);
//      } else {
//         handlerit++;
//      }
//   }
//}