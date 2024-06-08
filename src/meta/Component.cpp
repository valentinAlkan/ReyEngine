#include "Component.h"
#include "Application.h"

using namespace ReyEngine;
///////////////////////////////////////////////////////////////////////////////////////////
Component::Component(const std::string &name)
: _resourceId("resourceId", Application::instance().getNewRid())
, _isProcessed("isProcessed")
, _name(name)
{}

///////////////////////////////////////////////////////////////////////////////////////////
uint64_t Component::getFrameCounter() const {
   return Application::instance().getWindow(0)->getFrameCounter();
}