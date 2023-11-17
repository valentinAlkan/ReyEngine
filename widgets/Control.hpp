#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Control : public BaseWidget {

public:
   GFCSDRAW_OBJECT(Control, BaseWidget){}
public:
   void render() const override {
      if (renderCallback){
         renderCallback();
      }
   }
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
   void setRenderCallback(std::function<const void()> fx){
      renderCallback = fx;
   }
protected:
   std::function<void()> renderCallback;
};