#include "Control.h"
//#include "Application.h"

class TextureTestWidget : public ReyEngine::BaseWidget{
   REYENGINE_OBJECT_BUILD_ONLY(TextureTestWidget, BaseWidget){}
public:
   REYENGINE_DEFAULT_BUILD
   void renderBegin(ReyEngine::Pos<double>& textureOffset) override {}
   void render() const override {}
//   void renderEnd() override{}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
protected:
   ReyEngine::RenderTarget target;
};