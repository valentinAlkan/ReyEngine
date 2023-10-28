#include "Control.hpp"
//#include "Application.h"

class TextureTestWidget : public Control{
   GFCSDRAW_OBJECT(TextureTestWidget, Control)
   , target(getRect().size()){}
public:
   void renderBegin(GFCSDraw::Pos<double>& textureOffset) override {
//      _render
   }
   void render() const override {}
//   void renderEnd() override{}
   void _process(float dt) override {}
   void registerProperties() override{
      //register properties specific to your type here.
   }
protected:
   GFCSDraw::RenderTarget target;
};