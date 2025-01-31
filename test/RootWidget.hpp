#pragma once
#include "Control.h"

namespace ReyEngine{
   class RootWidget : public BaseWidget{
      REYENGINE_OBJECT_BUILD_ONLY(RootWidget, BaseWidget){}
   public:
      REYENGINE_DEFAULT_BUILD(RootWidget)
//      static std::shared_ptr<RootWidget> build(const std::string& name){return std::shared_ptr<RootWidget>(new RootWidget(name));}
   //   void renderBegin() override {}
      void render() const override {
         drawRectangle(getRect(), theme->background.colorPrimary.value);
      }
   //   void renderEnd() override{}
      void _process(float dt) override {}
      void registerProperties() override{
         //register properties specific to your type here.
      }
   };
}