#pragma once
#include "Component.h"
#include "Positionable2D.h"
#include <set>

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

         template <typename T, typename R=R_FLOAT>
         static std::shared_ptr<T> make_collider(const std::string& name, const Positionable2D<R>& positionable) {
            return std::shared_ptr<T>(new T(name, positionable), colliderDtor<T>);
         }

         static std::shared_ptr<Collision::CollisionRect> make_rect(const std::string& instanceName, const Positionable2D<R_FLOAT>& positionable);
         bool getIsColliding(){return _isColliding;}
      protected:
         template<typename T>
         static void colliderDtor(T* ptr);

         Collider(const std::string& instanceName, const std::string& typeName, const Positionable2D<R_FLOAT>& positionable)
         : Internal::Component(instanceName, typeName)
         , NamedInstance(instanceName, typeName)
         , positionable(positionable)
         {}

         const Positionable2D<R_FLOAT>& positionable;
         bool _isColliding = false;
         template<typename T, typename U>
         friend class CollisionChecker;
         friend class CollisionLayer;
      };

      class CollisionRect : public Collider {
      public:
         static constexpr char TYPE_NAME[] = "CollisionRect";
         CollisionRect(const std::string& instanceName, const Positionable2D<R_FLOAT>& positionable)
         : Collider(instanceName, TYPE_NAME, positionable)
         , NamedInstance(instanceName, TYPE_NAME)
         {}
      protected:
         // Get corners of a rectangle after transformation




         // Check if ranges overlap
         static bool rangesOverlap(float minA, float maxA, float minB, float maxB) {
            return minA <= maxB && maxA >= minB;
         }



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
         void checkLayer(int layer);
         std::map<CollisionRect*, std::set<CollisionRect*>> collisions;
         bool isColliding(CollisionRect* a, CollisionRect* b) const; //check if a is currently colliding with b
         bool isColliding(CollisionRect* a) const; //check if a is currently colliding with anything
      };
   }
}