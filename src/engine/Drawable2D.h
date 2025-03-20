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
         [[nodiscard]] inline Transform2D& getTransform(){return transform2D;}
         [[nodiscard]] inline std::optional<Window*> getWindow(){if (window) { return window; } else {return std::nullopt;}}
         [[nodiscard]] inline std::optional<const Window*> getWindow() const {if (window) { return window; } else {return std::nullopt;}}
         [[nodiscard]] inline std::optional<Canvas*> getCanvas(){if (canvas) { return canvas; } else {return std::nullopt;}}
         [[nodiscard]] inline std::optional<const Canvas*> getCanvas() const {return const_cast<Drawable2D*>(this)->getCanvas();}
         [[nodiscard]] inline bool getVisible(){return _visible;}
         inline void setVisible(bool newValue){_visible = newValue;}
      protected:
         void render2DChain();
         virtual void render2D() const = 0;
         virtual void render2DBegin(){}
         virtual void render2DEnd(){}
         bool _visible = true;
         Window* window = nullptr;
      private:
         Canvas* canvas = nullptr;
      friend class ReyEngine::Canvas;
      };
   }
}