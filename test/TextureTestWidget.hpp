#include "Control.hpp"
//#include "Application.h"

class TextureTestWidget : public Control{
   REYENGINE_OBJECT(TextureTestWidget, Control){}
public:
   void renderBegin(ReyEngine::Pos<double>& textureOffset) override {
//      _render
   }
   void render() const override {}
//   void renderEnd() override{}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
protected:
   ReyEngine::RenderTarget target;
};