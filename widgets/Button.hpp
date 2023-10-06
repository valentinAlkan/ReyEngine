#pragma once
#include "Control.hpp"

class BaseButton : public Control {
   GFCSDRAW_OBJECT(BaseButton, Control){}
public:
   void registerProperties() override;
};

class PushButton : public BaseButton{
   GFCSDRAW_OBJECT(PushButton, BaseButton)
   , PROPERTY_DECLARE(text){}
   void registerProperties() override {};
   void render() const override {}
   StringProperty text;

};

