#pragma once
#include "ReyObject.h"

namespace ReyEngine::Internal {
   class Positionable2D : public TypeTag {
   public:
      [[nodiscard]] inline Rect<R_FLOAT> getRect() const {return {transform2D.extractTranslation(), size};}
      [[nodiscard]] inline Size<R_FLOAT> getSize() const {return size;}
      [[nodiscard]] inline Pos<R_FLOAT> getPosition() const {return transform2D.extractTranslation();}
      [[nodiscard]] inline Radians getRotation() const {return transform2D.extractRotation();}
      [[nodiscard]] inline Vec2<R_FLOAT> getScale() const {return transform2D.extractScale();}

      inline void setPosition(const Pos<R_FLOAT>& newPosition){transform2D.setPosition(newPosition);}
      inline void setRotation(const Radians& newRotation){transform2D.rotate(newRotation);}
      inline void setScale(const Vec2<R_FLOAT>& newScale ){transform2D.scale(newScale);}
      inline void setSize(const Size<R_FLOAT>& newSize ){size = newSize;}
      inline Transform2D& getLocalTransform(){return transform2D;}
      inline Transform2D& getGlobalTransform(){return globalTransform;}
   protected:
      Transform2D  globalTransform;
      Transform2D transform2D;
      Size<float> size;
   };
}