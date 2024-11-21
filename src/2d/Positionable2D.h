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
      Positionable2D(Rect<T>& target) //don't make this const, want to reject temporaries
      : target(target)
      {}
      Pos<T> getPos() const {return target.pos();}
      Pos<T> getGlobalPos() const {
         //sum up all our ancestors' positions and add our own to it
         auto offset = getPos();
         if (parent){ //todo: Race conditions?
            offset += parent->getGlobalPos();
         }
         return offset;
      }
      ReyEngine::Rect<T> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
      ReyEngine::Rect<T> getRect() const {return target;}
      ReyEngine::Size<T> getSize() const {return target.size();}
      T getWidth() const {return target.width;}
      T getHeight() const {return target.height;}
      Vec2<T> getHeightRange() const {return {0, target.size().y};}
      Vec2<T> getWidthtRange() const {return {0, target.size().x};}
      template <typename C>
      std::shared_ptr<C> createCollider(const std::string& instanceName){
         std::cout << "Positionable: final address of positioanbnle rect.value hsould be: " << &target << std::endl;
         auto collider = C::template make_collider<C>(instanceName, *this);
         std::cout << "Positionable: however, the address of collider rect.value rect is " << &collider->positionable.target << std::endl;
         assert(&target == &collider->positionable.target);
         return collider;
      }
      void* getAddr() const {return (void*)&target;}
//      std::shared_ptr<Collision::CollisionRect> make_collider(const std::string& instanceName);
      Rect<R_FLOAT>& target; //always valid, never changes
   protected:
      void setParent(Positionable2D* newParent){parent=newParent;}
   private:
      Positionable2D* parent = nullptr; //potentially invalid if orphaned - non-owning
   };


}