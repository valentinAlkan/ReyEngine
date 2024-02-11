#include "Component.h"
#include "Application.h"

Component::Component(const std::string &name)
: _resourceId("resourceId", Application::instance().getNewRid())
, _isProcessed("isProcessed")
, _name(name)
{}