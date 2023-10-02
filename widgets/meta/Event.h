#pragma once
#include <memory>
#include <string>
class BaseWidget;
class Event {
public:
   Event(std::string eventType, std::shared_ptr<BaseWidget> publisher)
   : eventType(eventType)
   , publisher(publisher){}
   std::shared_ptr<BaseWidget> publisher;
   std::string eventType;
};
