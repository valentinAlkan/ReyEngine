#pragma once
#include "ReyObject.h"

namespace ReyEngine::Internal {
   class Positionable2D : public TypeTag {
   public:
      Positionable2D()
      : maxSize({std::numeric_limits<R_FLOAT>::max()})
      {}
      [[nodiscard]] inline Rect<R_FLOAT> getRect() const {return {transform2D.extractTranslation(), size};}
      [[nodiscard]] inline Size<R_FLOAT> getSize() const {return size;}
      [[nodiscard]] inline Pos<R_FLOAT> getPosition() const {return transform2D.extractTranslation();}
      [[nodiscard]] inline Radians getRotation() const {return transform2D.extractRotation();}
      [[nodiscard]] inline Vec2<R_FLOAT> getScale() const {return transform2D.extractScale();}

      inline void setPosition(const Pos<R_FLOAT>& newPosition){
         if (isLocked) return;
         auto r = getRect();
         transform2D.setPosition(newPosition);
         __on_rect_changed(r);
      }
      inline void setRotation(const Radians& newRotation){
         if (isLocked) return;
         auto r = getRect();
         transform2D.rotate(newRotation);
         __on_rect_changed(r);
      }
      inline void setScale(const Vec2<R_FLOAT>& newScale ){
         if (isLocked) return;
         auto r = getRect();
         transform2D.scale(newScale);
         __on_rect_changed(r);
      }
      inline void setSize(const Size<R_FLOAT>& newSize ){
         if (isLocked) return;
         auto r = getRect();
         size = newSize;
         __on_rect_changed(r);
      }
      inline void setRect(const Rect<R_FLOAT>& newRect){
         if (isLocked) return;
         size = newRect.size();
         auto r = getRect();
         transform2D.setPosition(newRect.pos());
         __on_rect_changed(r);
      }
      inline Transform2D& getLocalTransform(){return transform2D;}
//      inline Transform2D& getGlobalTransform(){return globalTransform;}
      inline Size<R_FLOAT> getMinSize(){return minSize;}
      inline Size<R_FLOAT> getMaxSize(){return maxSize;}
      inline void setMinSize(const Size<float>& newMin){minSize = newMin;}
      inline void setMaxSize(const Size<float>& newMax){maxSize = newMax;}
      inline R_FLOAT getWidth(){return size.x;}
      inline R_FLOAT getHeight(){return size.y;}
      inline Size<R_FLOAT> clampedSize(const Size<R_FLOAT>& rhs) {
         auto newX = ReyEngine::Vec2<R_FLOAT>(minSize.x, maxSize.x).clamp(size.x);
         auto newY = ReyEngine::Vec2<R_FLOAT>(minSize.y, maxSize.y).clamp(size.y);
         return {newX, newY};
      }

   protected:
      /// Bypasses normal mechanisms and just directly sets a rect without calling callbacks
      void applySize(const Size<R_FLOAT>& newSize){size = newSize;}
      void applyRect(const Rect<R_FLOAT>& rect){
         transform2D.setPosition(rect.pos());
         size = rect.size();
      }

      virtual void __on_rect_changed(const Rect<R_FLOAT>& oldRect){}; //internal. Trigger resize for anchored widgets.

      Transform2D  globalTransform;
      Transform2D transform2D;
      Size<float> size;
      Size<float> minSize;
      Size<float> maxSize;
      bool isLocked = false;
   };
}