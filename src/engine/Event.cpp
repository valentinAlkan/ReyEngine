#include "Event.h"

using namespace ReyEngine;

EventPublisher::~EventPublisher() {
   //notify all customers that their subscriptions are cancelled
   //NO REFUNDS
   for (auto it=_eventMap.begin(); it!=_eventMap.end();/**/){
      auto& callbackmap = it->second;
      for (auto& [subscriber, cbvec] : callbackmap){
         subscriber->cancelSubscription(this, true);
      }
      if (callbackmap.empty()){
         it = _eventMap.erase(it);
      } else {
         ++it;
      }
   }
}