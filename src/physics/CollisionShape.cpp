#include "CollisionShape.h"
#include "Physics.h"

using namespace std;
using namespace ReyEngine;
using namespace Collision;

//-------------------
// Get axis projection range of a set of points
void projectPoints(const std::array<Vec2<R_FLOAT>, 4>& points, const Vec2<R_FLOAT>& axis, float& min, float& max) {
   min = max = (points[0].x * axis.x + points[0].y * axis.y);

   for(int i = 1; i < 4; i++) {
      float projection = points[i].x * axis.x + points[i].y * axis.y;
      min = std::min(min, projection);
      max = std::max(max, projection);
   }
}

//-------------------
bool rangesOverlap(float minA, float maxA, float minB, float maxB) {
   return minA <= maxB && maxA >= minB;
}

static void drawCorners(const std::array<Vec2<R_FLOAT>, 4>& corners, Color color) {
   for(int i = 0; i < 4; i++) {
      // Draw corner points
      DrawCircle(corners[i].x, corners[i].y, 3.0f, color);

      // Draw edges
      DrawLineV(
            {corners[i].x, corners[i].y},
            {corners[(i+1)%4].x, corners[(i+1)%4].y},
            color
      );
   }
}

static void drawAxis(const Vec2<R_FLOAT>& origin, const Vec2<R_FLOAT>& axis, float length, Color color) {
   auto endPoint = origin + axis * length;
   DrawLineV({origin.x, origin.y}, {endPoint.x, endPoint.y}, color);

   // Draw arrow head
   auto normalized = axis.normalize();
   Vec2<R_FLOAT> perpendicular(-normalized.y, normalized.x);
   float arrowSize = 10.0f;

   auto arrowLeft = endPoint - normalized * arrowSize + perpendicular * (arrowSize * 0.5f);
   auto arrowRight = endPoint - normalized * arrowSize - perpendicular * (arrowSize * 0.5f);

   DrawLineV({endPoint.x, endPoint.y}, {arrowLeft.x, arrowLeft.y}, color);
   DrawLineV({endPoint.x, endPoint.y}, {arrowRight.x, arrowRight.y}, color);
}

static void drawProjection(const Vec2<R_FLOAT>& axis, float min, float max, float yOffset, Color color) {
   auto normalized = axis.normalize();
   auto perpendicular = Vec2<R_FLOAT>(-normalized.y, normalized.x);

   auto start = normalized * min + perpendicular * yOffset;
   auto end = normalized * max + perpendicular * yOffset;

   DrawLineV({start.x, start.y}, {end.x, end.y}, color);
   DrawCircle(start.x, start.y, 3.0f, color);
   DrawCircle(end.x, end.y, 3.0f, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Specialize for each pair
template<>
struct Collision::CollisionChecker<CollisionRect, CollisionRect> {
   static bool collide(const CollisionRect& a, const CollisionRect& b) {
      if (&a == &b) return false;

      cout << "Physics collision A matrix: " << endl;
      auto xformA = a.positionable.getGlobalTransformationMatrix();
      printMatrix(xformA);
      cout << "Physics collision B matrix: " << endl;
      auto xformB = b.positionable.getGlobalTransformationMatrix();
      printMatrix(xformB);

      auto _a = Vec2<R_FLOAT>{10,10};
      auto _b = Vec2<R_FLOAT>{5,20};
      cout << "Point " << _a << " transformed to " << _a.transform(xformA) << endl;
      cout << "Point " << _b << " transformed to " << _b.transform(xformB) << endl;

      auto rectA = a.positionable.getRect();
      auto rectB = b.positionable.getRect();

      // Get transformed corners using your transform method
      auto cornersA = rectA.transform(xformA);
      auto cornersB = rectB.transform(xformB);

      // Debug visualization of corners
      drawCorners(cornersA, RED);
      drawCorners(cornersB, BLUE);

      // Get axes to test
      std::array<Vec2<R_FLOAT>, 4> axes = {
            Vec2<R_FLOAT>(xformA.m0, xformA.m1),  // First column = x axis of A
            Vec2<R_FLOAT>(xformA.m4, xformA.m5),  // Second column = y axis of A
            Vec2<R_FLOAT>(xformB.m0, xformB.m1),  // First column = x axis of B
            Vec2<R_FLOAT>(xformB.m4, xformB.m5)   // Second column = y axis of B
      };

      // Normalize axes
      for(auto& axis : axes) {
         axis = axis.normalize();
      }

      // Test each axis
      Vec2<R_FLOAT> center(
            (cornersA[0].x + cornersA[2].x) * 0.5f,
            (cornersA[0].y + cornersA[2].y) * 0.5f
      );

      for(int i = 0; i < 4; i++) {
         const auto& axis = axes[i];

         // Draw the current axis being tested
         drawAxis(center, axis, 100.0f, GREEN);

         float minA, maxA, minB, maxB;
         projectPoints(cornersA, axis, minA, maxA);
         projectPoints(cornersB, axis, minB, maxB);

         // Draw projections with different offsets for visibility
         drawProjection(axis, minA, maxA, 20.0f, RED);
         drawProjection(axis, minB, maxB, -20.0f, BLUE);

         if(!rangesOverlap(minA, maxA, minB, maxB)) {
            return false;
         }
      }

      return true;
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

