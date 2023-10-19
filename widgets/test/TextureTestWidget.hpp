#include "Control.hpp"
//#include "Application.h"

class TextureTestWidget : public Control{
   GFCSDRAW_OBJECT(TextureTestWidget, Control)
   , target(_rect.value.size()){}
public:
   void renderBegin(GFCSDraw::Vec2<float>& textureOffset) override {
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