#pragma once
#include "Component.h"

namespace ReyEngine {
   namespace Collision{

      template<typename T, typename U>
      struct CollisionChecker;

      /// A shape which can collide with other shapes. Doesn't do much else.
      class Collider : public Internal::Component {
      public:
         struct CollisionEvent : public Event<CollisionEvent>{
            EVENT_CTOR_SIMPLE(CollisionEvent, Event<CollisionEvent>, const Collider& other), other(other){}
            const Collider& other;
         };
         template <typename T>
         bool collidesWith(const T& other) const {
            auto* derived = static_cast<const T*>(this);
            return CollisionChecker<std::decay_t<decltype(*derived)>, T>::collide(*derived, other);
         }
      protected:
         Collider(const std::string& instanceName, const std::string& typeName)
         : Internal::Component(instanceName, typeName)
         , NamedInstance(instanceName, typeName)
         {}
      };

      class CollisionRect : public Collider {
      public:
         static constexpr char TYPE_NAME[] = "CollisionRect";
         CollisionRect(const std::string& instanceName)
         : Collider(instanceName, TYPE_NAME)
         , NamedInstance(instanceName, TYPE_NAME)
         , PROPERTY_DECLARE(rect)
         {}
         RectProperty<R_FLOAT> rect;
      };

      class CollisionCircle : public Collider {
      public:
         static constexpr char TYPE_NAME[] = "CollisionCircle";
         CollisionCircle(const std::string& instanceName)
         : Collider(instanceName, TYPE_NAME)
         , NamedInstance(instanceName, TYPE_NAME)
         , PROPERTY_DECLARE(circle)
         {}
         CircleProperty circle;
      };

      // Every shape in a layer can collide with every other shape in this layer
      struct CollisionLayer{
         std::vector<CollisionRect*> rects;
         std::vector<CollisionCircle*> circles;
         void checkLayer() const;
      };

   }
}