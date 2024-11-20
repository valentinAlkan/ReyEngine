#include "CollisionShape.h"
#include "Physics.h"

using namespace ReyEngine;
using namespace Collision;

/////////////////////////////////////////////////////////////////////////////////////////
// Specialize for each pair
template<>
struct Collision::CollisionChecker<CollisionRect, CollisionRect> {
   static bool collide(const CollisionRect& a, const CollisionRect& b) {
      return a.positionable.getGlobalRect().collides(b.positionable.getGlobalRect());
   }
};

std::shared_ptr<Collision::CollisionRect> Collider::make_rect(const std::string &instanceName, const Positionable2D<float> &positionable) {
   auto dtor = [&](CollisionRect* ptr) {
      for (auto layer: ptr->layers) {
         Physics::PhysicsSystem::removeColliderFromLayer(layer, *ptr);
      }
   };
   return {new Collision::CollisionRect(instanceName, positionable), dtor };
}

//
///////////////////////////////////////////////////////////////////////////////////////////
//template<>
//struct Collision::CollisionChecker<CollisionCircle, CollisionCircle> {
//   static bool collide(const CollisionCircle& a, const CollisionCircle& b) {
//      throw std::runtime_error("Not implemented - circles are postiionable?");
////      return a.circle.collides(b.circle);
//   }
//};
//
///////////////////////////////////////////////////////////////////////////////////////////
//template<>
//struct Collision::CollisionChecker<CollisionRect, CollisionCircle> {
//   static bool collide(const CollisionRect& a, const CollisionCircle& b) {
//      throw std::runtime_error("Not implemented - circles are postiionable?");
////      return b.circle.collides(b.circle);
//   }
//};
//
///////////////////////////////////////////////////////////////////////////////////////////
//template<>
//struct Collision::CollisionChecker<CollisionCircle, CollisionRect> {
//   static bool collide(const CollisionCircle& a, const CollisionRect& b) {
//      return CollisionChecker<CollisionRect, CollisionCircle>::collide(b, a);
//   }
//};


/////////////////////////////////////////////////////////////////////////////////////////
template <>
void Collider::addToLayer<CollisionRect>(int layer) {
   Physics::PhysicsSystem::addColliderToLayer(layer, dynamic_cast<CollisionRect&>(*this));
   layers.push_back(layer);
}

/////////////////////////////////////////////////////////////////////////////////////////
//template <>
//void Collider::addToLayer<CollisionCircle>(int layer) {
//   Physics::PhysicsSystem::addColliderToLayer(layer, dynamic_cast<CollisionCircle&>(*this));
//   layers.push_back(layer);
//}

/////////////////////////////////////////////////////////////////////////////////////////
template <>
void Collider::colliderDtor(CollisionRect* ptr) {
   for (auto layer : ptr->layers) {
      Physics::PhysicsSystem::removeColliderFromLayer(layer, *ptr);
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
void Collision::CollisionLayer::checkLayer(int layer) const {
   auto checkSecond = [](auto& first, const auto& seconds, int layer){
      for (const auto& second : seconds){
         if (first.collidesWith(*second)){
            Collider::CollisionEvent event(first.EventPublisher::shared_from_this(), *second, layer);
            first.publish(event);
         }
      }
   };

   for (const auto& first : rects){
      checkSecond(*first, rects, layer);
//      checkSecond(*first, circles, layer);
   }
//   for (const auto& first : circles){
//      checkSecond(*first, circles, layer);
//   }
}