#pragma once
#include "Positionable2D.h"

namespace ReyEngine {
   class Canvas;
   namespace Internal {
      class Drawable2D
         : public ReyObject
         , public Positionable2D {
      public:
         REYENGINE_OBJECT(Drawable2D)
         ~Drawable2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
         [[nodiscard]] ReyEngine::Rect<float> getRect() const {return {getPosition(), getSize()};}
         [[nodiscard]] ReyEngine::Size<float> getSize() const {return size;}
         Transform2D& getTransform(){return transform2D;}
      protected:
         void render2DChain();
         virtual void render2D() const = 0;
         virtual void render2DBegin(){}
         virtual void render2DEnd(){}
         bool _visible = true;
      private:
         Canvas* canvas = nullptr;
      friend class ReyEngine::Canvas;
      };
   }
}