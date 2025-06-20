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
      [[nodiscard]] inline Rect<R_FLOAT> getSizeRect() const {return {0,0,size.x, size.y};}
      [[nodiscard]] inline Pos<R_FLOAT> getPos() const {return transform2D.extractTranslation();}
      [[nodiscard]] inline Radians getRotation() const {return transform2D.extractRotation();}
      [[nodiscard]] inline Vec2<R_FLOAT> getScale() const {return transform2D.extractScale();}

      inline void setPosition(const Pos<R_FLOAT>& newPosition){
         if (isLocked) return;
         auto r = getRect();
         transform2D.setPosition(newPosition);
         __on_rect_changed(r);
      }
      inline void setPosition(R_FLOAT x, R_FLOAT y){setPosition({x, y});}
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
      inline void setSize(R_FLOAT x, R_FLOAT y){setSize({x, y});}
      inline void setRect(const Rect<R_FLOAT>& newRect){
         if (isLocked) return;
         size = newRect.size();
         auto r = getRect();
         transform2D.setPosition(newRect.pos());
         __on_rect_changed(r);
      }
      inline void setWidth(R_FLOAT x){setSize({x, getHeight()});}
      inline void setHeight(R_FLOAT y){setSize({getWidth(), y});}
      inline void setRect(R_FLOAT x, R_FLOAT y, R_FLOAT width, R_FLOAT height){setRect({x,y,width,height});}
      inline Transform2D& getLocalTransform(){return transform2D;}
      inline const Transform2D& getLocalTransform() const {return transform2D;}
      inline CanvasSpace<Transform2D> getGlobalTransform(bool respectGlobalTransformBoundary=true) const {
         auto retval = getLocalTransform();
         auto parent = selfNode->getParent();
         while (parent){
            if (auto isPositionable = parent->tag<Positionable2D>()){
               //break on global transform boundaries - they are NOT part of the global transform for children
               if (isPositionable.value()->isGlobalTransformBoundary && respectGlobalTransformBoundary){
                  break;
               }
               retval *= isPositionable.value()->transform2D;
            }
            parent = parent->getParent();
         }
         return retval;
      }
      inline CanvasSpace<Rect<R_FLOAT>> getGloablRect(){
         auto gpos = getGlobalTransform().get().extractTranslation();
         Rect<R_FLOAT> rect = {gpos,size};
         return rect;
      }
      inline Size<R_FLOAT> getMinSize() const {return minSize;}
      inline Size<R_FLOAT> getMaxSize() const {return maxSize;}
      inline void setMinSize(const Size<float>& newMin){minSize = newMin; setSize(getSize());}
      inline void setMaxSize(const Size<float>& newMax){maxSize = newMax; setSize(getSize());}
      inline void setMaxHeight(float newMax){maxSize = {maxSize.x, newMax}; setSize(getSize());}
      inline void setMaxWidth(float newMax){maxSize = {newMax, maxSize.y}; setSize(getSize());}
      inline void setMinWidth(float newMin){minSize = {newMin, minSize.y}; setSize(getSize());}
      inline void setMinHeight(float newMin){minSize = {minSize.x, newMin}; setSize(getSize());}
      inline R_FLOAT getWidth() const {return size.x;}
      inline R_FLOAT getHeight() const {return size.y;}
      inline Pos<float> getCenter() const {return getRect().center();}
      inline Size<R_FLOAT> clampedSize(const Size<R_FLOAT>& rhs) const {
         auto newX = ReyEngine::Vec2<R_FLOAT>(minSize.x, maxSize.x).clamp(size.x);
         auto newY = ReyEngine::Vec2<R_FLOAT>(minSize.y, maxSize.y).clamp(size.y);
         return {newX, newY};
      }
      bool isInside(const Pos<R_FLOAT>& pos) const { return getSizeRect().contains(pos);}

   protected:
      /// Bypasses normal mechanisms and just directly sets a rect without calling callbacks
      void applySize(const Size<R_FLOAT>& newSize){size = newSize;}
      void applyRect(const Rect<R_FLOAT>& rect){
         transform2D.setPosition(rect.pos());
         size = rect.size();
      }

      virtual void __on_rect_changed(const Rect<R_FLOAT>& oldRect, bool byLayout=false){}; //internal. Trigger resize for anchored widgets.

//      Transform2D  globalTransform;
      Transform2D transform2D;
      Size<float> size;
      Size<float> minSize;
      Size<float> maxSize;
      bool isLocked = false;
      //when this is true, do not propogate global transform requests any further up the chain.
      // For the most part, this should only be true for canvases.
      bool isGlobalTransformBoundary = false;
   private:
      Positionable2D* getParentPositionable(bool respectGlobalTransformBoundary = true) {
         auto parent = selfNode->getParent();
         while (parent) {
            if (auto isPositionable = parent->tag<Positionable2D>()) {
               return isPositionable.value();
            }
            parent = parent->getParent();
         }
         return nullptr;
      }
      void triggerParentResize(){
         if (auto hasParent = getParentPositionable()) {
            getParentPositionable()->setSize(hasParent->getSize());
         }
      }
   };
}