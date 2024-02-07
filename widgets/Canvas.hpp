#pragma once
#include "Control.hpp"

class Canvas : public Control{

public:
   REYENGINE_OBJECT(Canvas, Control)
   {}
public:
   void renderBegin(ReyEngine::Vec2<float>& textureOffset) override {

   }
   void render() const override {

   }
   void renderEnd() override{

   }
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
   ReyEngine::RenderTarget target;
};