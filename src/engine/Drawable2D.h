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
         [[nodiscard]] inline Transform2D& getTransform(){return transform2D;}
         [[nodiscard]] inline Transform2D getTransform() const {return transform2D;}
         [[nodiscard]] inline std::optional<Canvas*> getCanvas(){if (canvas) { return canvas; } else {return std::nullopt;}}
         [[nodiscard]] inline std::optional<const Canvas*> getCanvas() const {return const_cast<Drawable2D*>(this)->getCanvas();}
         [[nodiscard]] inline bool getVisible(){return _visible;}
         inline void setVisible(bool newValue){ _visible = newValue; __on_visibility_changed();}
      protected:
         virtual void render2D() const = 0;
         virtual void render2DBegin(){}
         virtual void render2DEnd(){}
         virtual void __on_visibility_changed(){}
         bool _visible = true;
         bool _isCanvas = false; //whether or not this drawable is a canvas
      private:
         Canvas* canvas = nullptr;
      friend class ReyEngine::Canvas;
      };
   }
}