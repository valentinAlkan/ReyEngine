#pragma once
#include <optional>
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
         virtual void render2D(RenderContext&) const = 0;
         virtual void render2DBegin(RenderContext&){}
         virtual void render2DEnd(RenderContext&){}
         /// Region, in this drawable's local space, that its children are clipped to.
         /// nullopt (the default) means no clipping.
         ///
         /// This exists because children are rendered *after* render2DEnd, so a ScopeScissor
         /// held across render2DBegin/render2DEnd is already gone by the time they draw. The
         /// region returned here stays applied for the whole child subtree, so it constrains
         /// grandchildren too. Our own render2D is NOT clipped by it - only descendants are.
         virtual std::optional<Rect<R_FLOAT>> getChildClipRect() const {return std::nullopt;}
         virtual void __on_visibility_changed(){}
         bool _visible = true;
         bool _isCanvas = false; //whether or not this drawable is a canvas
      private:
         Canvas* canvas = nullptr;
      friend class ReyEngine::Canvas;
      };
   }
}