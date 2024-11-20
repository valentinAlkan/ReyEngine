#pragma once
#include "Component.h"
#include "Positionable2D.h"

namespace ReyEngine {
   namespace Collision{

      template<typename T, typename U>
      struct CollisionChecker;

      /// A shape which can collide with other shapes. Doesn't do much else.
      class Collider : public Internal::Component {
      public:
         struct CollisionEvent : public Event<CollisionEvent>{
            EVENT_CTOR_SIMPLE(CollisionEvent, Event<CollisionEvent>, const Collider& other, const int layer)
            , other(other)
            , layer(layer)
            {}
            const Collider& other;
            const int layer;
         };
         template <typename T>
         bool collidesWith(const T& other) const {
            auto* derived = static_cast<const T*>(this);
            return CollisionChecker<std::decay_t<decltype(*derived)>, T>::collide(*derived, other);
         }
         template <typename T>
         void addToLayer(int collisionLayer);
         std::vector<int> layers;

//         template <typename T, typename... Args>
//         static std::shared_ptr<T> make_collider(Args &&... args) {
//            return std::shared_ptr<T>(new T(std::forward<Args>(args)...), colliderDtor<T>);
//         }
         template <typename T, typename R=R_FLOAT>
         static std::shared_ptr<T> make_collider(const std::string& name, const Positionable2D<R>& positionable) {
            std::cout << "make_Collider: address of target is " << &positionable.target << std::endl;
            auto ptr = std::shared_ptr<T>(new T(name, positionable), colliderDtor<T>);
            std::cout << "make_Collider: address of collider target is " << &ptr->positionable.target << std::endl;
            std::cout << "make_Collider: value of positioanolbeTarge is " << positionable.target << std::endl;
            return ptr;
         }

         static std::shared_ptr<Collision::CollisionRect> make_rect(const std::string& instanceName, const Positionable2D<R_FLOAT>& positionable);
         void* getPositionableTargetAddr() const {
            return (void*)&positionable.target;
         }

//      protected:
         template<typename T>
         static void colliderDtor(T* ptr);

         Collider(const std::string& instanceName, const std::string& typeName, const Positionable2D<R_FLOAT>& positionable)
         : Internal::Component(instanceName, typeName)
         , NamedInstance(instanceName, typeName)
         , positionable(positionable)
         {}

         const Positionable2D<R_FLOAT>& positionable;

         template<typename T, typename U>
         friend class CollisionChecker;
      };

      class CollisionRect : public Collider {
      public:
         static constexpr char TYPE_NAME[] = "CollisionRect";
         CollisionRect(const std::string& instanceName, const Positionable2D<R_FLOAT>& positionable)
         : Collider(instanceName, TYPE_NAME, positionable)
         , NamedInstance(instanceName, TYPE_NAME)
         {
            std::cout << "Hello from collision rect ctor! target rect address is " << positionable.getAddr() << std::endl;
         }
      protected:
         friend class Collider;
      };

//      class CollisionCircle : public Collider {
//      protected:
//         static constexpr char TYPE_NAME[] = "CollisionCircle";
//         CollisionCircle(const std::string& instanceName, Positionable2D<R_FLOAT>& positionable)
//         : Collider(instanceName, TYPE_NAME)
//         , NamedInstance(instanceName, TYPE_NAME)
//         , positionable(positionable)
//         {}
//      private:
//         Positionable2D<R_FLOAT>& positionable;
//         friend class Collider;
//      };

      // Every shape in a layer can collide with every other shape in this layer
      struct CollisionLayer{
         std::vector<CollisionRect*> rects;
//         std::vector<CollisionCircle*> circles;
         void checkLayer(int layer) const;
      };

   }
}