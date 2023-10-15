#pragma once
#include <memory>
#include <string>

#define DECLARE_EVENT(EVENT_TYPE_NAME) static constexpr char EVENT_TYPE_NAME[] = #EVENT_TYPE_NAME;

class BaseWidget;
class Event {
public:
   Event(const std::string& eventType)
   : eventType(eventType){}
   std::shared_ptr<BaseWidget> publisher;
   std::string eventType;
};
