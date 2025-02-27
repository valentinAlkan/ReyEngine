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
      void cacheDrawables();
      std::vector<std::pair<Matrix, Drawable2D*>> drawOrder;
      void renderProcess();
      void render2D() const override;
      void render2DBegin() override;
      void render2DEnd() override;

   protected:
      RenderTarget& getRenderTarget(){return renderTarget;}
   private:
      RenderTarget renderTarget;

      friend class Window2;
   };
}