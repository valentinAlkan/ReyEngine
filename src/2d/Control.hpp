#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Control : public BaseWidget {

public:
   REYENGINE_OBJECT(Control, BaseWidget){}
public:
   void render() const override {
      if (renderCallback){
         renderCallback();
      } else {
         if (theme->background.value == Style::Fill::SOLID) {
            _drawRectangle(getRect().toSizeRect(), theme->background.colorPrimary.value);
         }
      }
   }
   void _process(float dt) override {
      if (processCallback){
         processCallback();
      }
   }
   void registerProperties() override{
      //register properties specific to your type here.
   }
   void setRenderCallback(std::function<const void()> fx){
      renderCallback = fx;
   }
   void setProcessCallback(std::function<void()> fx){
      processCallback = fx;
      setProcess(true);
   }

protected:
   std::function<void()> renderCallback;
   std::function<void()> processCallback;
};