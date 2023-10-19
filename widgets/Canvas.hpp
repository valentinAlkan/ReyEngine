#pragma once
#include "Control.hpp"

class Canvas : public Control{

public:
   GFCSDRAW_OBJECT(Canvas, Control)
   {}
public:
   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {

   }
   void render() const override {

   }
   void renderEnd() override{

   }
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
   GFCSDraw::RenderTarget target;
};