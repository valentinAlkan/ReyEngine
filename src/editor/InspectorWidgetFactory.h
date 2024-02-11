#pragma once
#include "BaseWidget.h"

///A factory which generates widgets for the inspector so we can view the properties
class InspectorWidgetFactory {
public:
   static std::shared_ptr<BaseWidget> generateWidget(const BaseProperty&);
};