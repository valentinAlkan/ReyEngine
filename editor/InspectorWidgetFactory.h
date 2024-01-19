#pragma once
#include "BaseWidget.h"

///A singleton which generates widgets for the inspector so we can view the properties
class InspectorWidgetFactory {
public:
   static InspectorWidgetFactory& instance();
   std::shared_ptr<BaseWidget> generateWidget(const BaseProperty&) const;
private:
   static std::unique_ptr<InspectorWidgetFactory> _self;
};