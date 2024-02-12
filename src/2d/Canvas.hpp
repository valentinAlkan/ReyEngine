#pragma once
#include "BaseWidget.h"

class Canvas : public BaseWidget {

public:
REYENGINE_OBJECT(Canvas, BaseWidget)
, target()
{}
public:
   void renderBegin(ReyEngine::Pos<double>& textureOffset) override {
      target.beginRenderMode();
      ClearBackground(ReyEngine::Colors::none);
   }
   void render() const override {}
   void renderEnd() override{
      target.endRenderMode();
      target.render(getGlobalRect().pos());
   }
   void _on_rect_changed() override {
      target.setSize(_rect.value.size());
   }
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
   ReyEngine::RenderTarget target;
};