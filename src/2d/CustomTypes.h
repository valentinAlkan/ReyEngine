#pragma once
#include "TestWidgets.h"

#define REGISTER_CUSTOM_TYPE(UNIQUE_CLASS_NAME, PARENT_CLASS_NAME, IS_VIRTUAL) \
BaseWidget::registerType(#UNIQUE_CLASS_NAME, #PARENT_CLASS_NAME, IS_VIRTUAL, &UNIQUE_CLASS_NAME::deserialize); \

namespace ReyEngine::CustomTypes{
   void registerTypes();
}