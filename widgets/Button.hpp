#pragma once
#include "Control.hpp"

class BaseButton : public Control {
   GFCSDRAW_OBJECT_SIMPLE(BaseButton, Control){}
public:
   void registerProperties() override;
};

class PushButton : public BaseWidget{

};

