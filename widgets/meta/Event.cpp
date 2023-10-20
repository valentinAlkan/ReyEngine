#include "Event.h"
#include "BaseWidget.h"
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<BaseWidget> EventPublisher::toBaseWidget() {
   auto me = EventPublisher::shared_from_this();
   return std::dynamic_pointer_cast<BaseWidget>(me);
}
