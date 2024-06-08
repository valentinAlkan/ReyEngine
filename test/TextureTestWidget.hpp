#include "Control.h"
//#include "Application.h"

class TextureTestWidget : public ReyEngine::BaseWidget{
   REYENGINE_OBJECT(TextureTestWidget, BaseWidget){}
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