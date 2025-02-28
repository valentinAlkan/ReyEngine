#pragma once
#include "Widget2.h"

namespace ReyEngine {
   class Canvas
      : public Widget2
      {
   public:
      REYENGINE_OBJECT(Canvas)
      Canvas(){}
      virtual ~Canvas() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
      void _on_descendant_added_to_tree(TypeNode* child) override;

      ///walk the tree and pin any drawables to us
      void cacheDrawables(size_t count);
      void renderProcess();
      void render2D() const override;
      void render2DBegin() override;
      void render2DEnd() override;
      CanvasSpace<Pos<float>> getMousePos();
      void updateGlobalTransforms();
   protected:
      //naive implementation for now. I assume there's a smarter way to do this
      // than backtracking all the way up a drawable's heirarchy and pushing the transformation matrices
      struct DrawOrderData {
         DrawOrderData(Drawable2D* drawable)
         : drawable(drawable)
         {}
         Drawable2D* drawable;
         size_t childCount;
      };

      RenderTarget& getRenderTarget(){return renderTarget;}
      std::vector<DrawOrderData> drawOrder;
   private:
      RenderTarget renderTarget;

      friend class Window2;
   };
}