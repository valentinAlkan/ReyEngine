#pragma once
#include "Positionable2D2.h"

namespace ReyEngine {
   class Canvas;
   namespace Internal {
      class Drawable2D : public ReyObject, public Positionable2D2 {
      public:
         REYENGINE_OBJECT(Drawable2D)
         Drawable2D(): Positionable2D2(){}
         ~Drawable2D() { std::cout << "Goodbye from " << TYPE_NAME << "!!" << std::endl; }
         [[nodiscard]] ReyEngine::Rect<float> getRect() const {return {getPosition(), getSize()};}
         [[nodiscard]] ReyEngine::Size<float> getSize() const {return size;}
      protected:
         virtual void render() = 0;
      private:
         Canvas* canvas = nullptr;
      };
   }
}