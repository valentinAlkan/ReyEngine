#include "Control.h"
//#include "Application.h"

class TextureTestWidget : public ReyEngine::BaseWidget{
   REYENGINE_OBJECT_BUILD_ONLY(TextureTestWidget, BaseWidget){}
public:
//   static std::shared_ptr<TextureTestWidget> build(const std::string& name){return std::shared_ptr<TextureTestWidget>(new TextureTestWidget(name));}
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