#pragma once

#include "Control.hpp"

class Slider : public Control {
   GFCSDRAW_OBJECT(Slider, Control),
   PROPERTY_DECLARE(value)
   {}
public:
   void registerProperties() override{
      Control::registerProperties();
   }
protected:
   FloatProperty value;
};

class VSlider : public Slider {
   GFCSDRAW_OBJECT(VSlider, Slider){}
public:
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{
      Slider::registerProperties();
   }
};

class HSlider : public Slider {
   GFCSDRAW_OBJECT(HSlider, Slider){}
public:
   void render() const override {}
   void _process(float dt) override {}
   void registerProperties() override{
      // YOU MUST CALL THE PARENT registerProperties FUNCTION! OR YOU DIE!
      // It is good practice to override it and call the parent even
      // if you have no properties to register just so you dont forget
      Slider::registerProperties();
      //register properties specific to your type here.
   }
};