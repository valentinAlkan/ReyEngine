#pragma once
#include "BaseWidget.h"

class BaseButton : public BaseWidget {
public:
   BaseButton(std:: string name);
   void registerProperties() override;

};