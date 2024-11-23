#pragma once
#include "DrawInterface.h"
#include "Property.h"

namespace ReyEngine{
   namespace Collision{
      class CollisionRect;
   }

   template <typename T>
   class Positionable2D {
   public:
      Positionable2D(Pos<R_FLOAT>& pos, Size<R_FLOAT>& size, Transform2D& targetTransform) //don't make this const, want to reject temporaries
      : pos(pos)
      , size(size)
      , transform(targetTransform)
      {}
      Pos<T> getPos() const {return pos;}
      Pos<T> getCenter() const {return pos + size / 2;}
      Pos<T> getGlobalPos() const {
         //sum up all our ancestors' positions and add our own to it
         auto sum = getPos();
         if (parent){ //todo: Race conditions?
            sum += parent->getGlobalPos();
         }
         return sum;
      }
      Transform2D getGlobalTransform() const {
         //sum up all our ancestors' rotations and add our own to it
         auto sum = transform;
         if (parent){ //todo: Race conditions?
            sum *= parent->transform;
         }
         return sum;
      }
      ReyEngine::Rect<T> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
      ReyEngine::Rect<T> getRect() const {return {pos, size};}
      ReyEngine::Size<T> getSize() const {return size;}
      Transform2D getTransform(){return transform;}
      Degrees getRotation(){return Radians(transform.rotation);}
      void setRotation(Degrees newRotation){transform.rotation = Radians(newRotation).get();}
      T getWidth() const {return size.x;}
      T getHeight() const {return size.y;}
      Vec2<T> getHeightRange() const {return {0, size.y};}
      Vec2<T> getWidthtRange() const {return {0, size.x};}
      template <typename C>
      std::shared_ptr<C> createCollider(const std::string& instanceName){
         return C::template make_collider<C>(instanceName, *this);
      }
//      std::shared_ptr<Collision::CollisionRect> make_collider(const std::string& instanceName);
      Pos<R_FLOAT>& pos;
      Size<R_FLOAT>& size;
      Transform2D& transform;
   protected:
      /// Bypasses normal mechanisms and just directly sets a rect
      void applySize(const Size<R_FLOAT>& newSize){size = newSize;}
      void applyRect(const Rect<R_FLOAT>& rect){
         pos = rect.pos();
         size = rect.size();
      }
      void setParent(Positionable2D* newParent){parent=newParent;}
   private:
      Positionable2D* parent = nullptr; //potentially invalid if orphaned - non-owning
   };


}