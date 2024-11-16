#include "CollisionShape.h"

using namespace ReyEngine;
using namespace Collision;

/////////////////////////////////////////////////////////////////////////////////////////
// Specialize for each pair
template<>
struct Collision::CollisionChecker<CollisionRect, CollisionRect> {
   static bool collide(const CollisionRect& a, const CollisionRect& b) {
      return a.rect.value.collides(b.rect.value);
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
template<>
struct Collision::CollisionChecker<CollisionCircle, CollisionCircle> {
   static bool collide(const CollisionCircle& a, const CollisionCircle& b) {
      return a.circle.value.collides(b.circle.value);
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
template<>
struct Collision::CollisionChecker<CollisionRect, CollisionCircle> {
   static bool collide(const CollisionRect& a, const CollisionCircle& b) {
      return b.circle.value.collides(b.circle.value);
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
template<>
struct Collision::CollisionChecker<CollisionCircle, CollisionRect> {
   static bool collide(const CollisionCircle& a, const CollisionRect& b) {
      return CollisionChecker<CollisionRect, CollisionCircle>::collide(b, a);
   }
};


/////////////////////////////////////////////////////////////////////////////////////////
void Collision::CollisionLayer::checkLayer() const {
   for (const auto& rect : rects){
      for (const auto& circle : circles){
         if (rect->collidesWith(*circle)){
            //publish collision event
//            Collider::CollisionEvent event(rect)
         }
      }
   }
}