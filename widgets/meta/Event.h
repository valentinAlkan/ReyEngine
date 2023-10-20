#pragma once
#include <memory>
#include <string>

#define EVENT_CTOR(CLASSNAME, PARENTCLASS, EVENT_TYPE_NAME) \
static constexpr char EVENT_TYPE_NAME[] = #EVENT_TYPE_NAME; \
CLASSNAME(const std::shared_ptr<BaseWidget>& publisher): PARENTCLASS(EVENT_TYPE_NAME, publisher){} \
CLASSNAME(const std::string& eventType, const std::shared_ptr<BaseWidget>& publisher): PARENTCLASS(eventType, publisher){}

class BaseWidget;


class BaseEvent{
public:
   explicit BaseEvent(const std::string& eventType)
   : eventType(eventType){}
   std::string eventType;
//   template <typename T>
//   T& toType() const {
//      static_assert(std::is_base_of_v<BaseEvent, T> == true);
//      return *(T*)this;
//   }
};

template <typename T>
class Event : public BaseEvent {
public:
   Event(const std::string& eventType, const std::shared_ptr<BaseWidget>& publisher)
   : BaseEvent(eventType)
   , publisher(publisher)
   {}
   const std::shared_ptr<BaseWidget>& publisher;
   std::string eventType;
};
