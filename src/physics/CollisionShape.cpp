#include "CollisionShape.h"
#include "Physics.h"

using namespace std;
using namespace ReyEngine;
using namespace Collision;

/////////////////////////////////////////////////////////////////////////////////////////
// Specialize for each pair
template<>
struct Collision::CollisionChecker<CollisionRect, CollisionRect> {
   static bool collide(const CollisionRect& a, const CollisionRect& b) {
      if (&a == &b) return false; //no self collisions
      auto rectA = a.positionable.getRect();
      auto rectB = b.positionable.getRect();
      return rectA.collides(rectB);
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
shared_ptr<Collision::CollisionRect> Collider::make_rect(const string &instanceName, const Positionable2D<float> &positionable) {
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
//      throw runtime_error("Not implemented - circles are postiionable?");
////      return a.circle.collides(b.circle);
//   }
//};
//
///////////////////////////////////////////////////////////////////////////////////////////
//template<>
//struct Collision::CollisionChecker<CollisionRect, CollisionCircle> {
//   static bool collide(const CollisionRect& a, const CollisionCircle& b) {
//      throw runtime_error("Not implemented - circles are postiionable?");
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
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Collision::CollisionLayer::checkLayer(int layer) {
   // never check the same collider against itself
//   auto catLayer = [&](){
//      for (const auto& first : collisions){
//         auto& secondSet = first.second;
//         stringstream ss;
//         for (const auto& second : secondSet) {
//            ss << &second << ",";
//         }
//         auto s = ss.str();
//         if (s.size()) {
//            cout << "Collider " << &first << " collides with " << ss.str() << endl;
//         }
//      }
//   };
   for (int i=0; i<rects.size(); i++){
      auto first = rects[i];
      for (int j=i+1; j<rects.size(); j++){
         auto second = rects[j];
         bool collidesNow = first->collidesWith(*second);
         bool wasColliding = isColliding(first, second);
         if (collidesNow && !wasColliding){
            //will overwrite if exists
            collisions[first].insert(second);
            collisions[second].insert(first);
            first->_isColliding = true;
            second->_isColliding = true;
            //publish events for each collider
            {
               //TODO: reorder this so that they are emitted AFTER collision variables are set on colliders? but we already know so maybe it doesn't matter
               Collider::CollisionEvent event(first->EventPublisher::shared_from_this(), *second, layer);
               first->publish(event);
            }
            {
               Collider::CollisionEvent event(second->EventPublisher::shared_from_this(), *first, layer);
               second->publish(event);
            }
//            catLayer();
         } else if (!collidesNow && wasColliding) {
            //remove from collision map - we know these entries exist so no need to verify
            auto& firstCollisions = collisions[first];
            auto& secondCollisions = collisions[second];
            firstCollisions.erase(second);
            secondCollisions.erase(first);
            if (firstCollisions.empty()){
               collisions.erase(first);
               first->_isColliding = false;
            }
            if (secondCollisions.empty()){
               collisions.erase(second);
               second->_isColliding = false;
            }
            //TODO: Collision exit events
//            catLayer();
         }
      }
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
bool Collision::CollisionLayer::isColliding(CollisionRect *a) const {
   auto foundA = collisions.find(a);
   return foundA != collisions.end();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Collision::CollisionLayer::isColliding(CollisionRect *a, CollisionRect *b) const {
   auto foundA = collisions.find(a);
   if (foundA == collisions.end()) return false;
   auto foundSet = foundA->second;
   auto foundB = foundSet.find(b);
   return foundB != foundSet.end();
}

