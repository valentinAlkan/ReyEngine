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
      Positionable2D(Size<R_FLOAT>& size, Transform2D& targetTransform) //don't make this const, want to reject temporaries
      : size(size)
      , transform(targetTransform)
      {}
      Pos<T> getPos() const {return transform.position;}
      Pos<T> getCenter() const {return transform.position + size / 2;}
      Pos<T> getGlobalPos() const {
         //sum up all our ancestors' positions and add our own to it
         auto sum = getPos();
         if (parent){ //todo: Race conditions?
            sum += parent->getGlobalPos();
         }
         return sum;
      }
      ReyEngine::Rect<T> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
      ReyEngine::Rect<T> getRect() const {return {transform.position, size};}
      ReyEngine::Size<T> getSize() const {return size;}
      Transform2D& getTransform(){return transform;}
      Matrix getTransformationMatrix() const {
         //Rotate around the position, do not rotate around {0,0}
         //column ordered translation
         Matrix f = MatrixIdentity();
         f = MatrixMultiply(f, MatrixScale(transform.scale.x, transform.scale.y, 1));
         f = MatrixMultiply(f, MatrixRotate({0,0,1}, transform.rotation));
         f = MatrixMultiply(f, MatrixTranslate(transform.position.x, transform.position.y, 0));
         return f;
      }
      Matrix getGlobalTransformationMatrix() const {
         auto f = getTransformationMatrix();
         if (parent){
            f = MatrixMultiply(f, parent->getGlobalTransformationMatrix());
         }
         return f;
      }
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
      Size<R_FLOAT>& size;
      Transform2D& transform;
   protected:
      /// Bypasses normal mechanisms and just directly sets a rect
      void applySize(const Size<R_FLOAT>& newSize){size = newSize;}
      void applyRect(const Rect<R_FLOAT>& rect){
         transform.position = rect.pos();
         size = rect.size();
      }
      void setParent(Positionable2D* newParent){parent=newParent;}
   private:
      Positionable2D* parent = nullptr; //potentially invalid if orphaned - non-owning
   };


}