#pragma once
#include <memory>
#include <set>
#include "CollisionShape.h"
#include "MiscTools.h"

namespace ReyEngine {
   namespace Physics {
      class PhysicsSystem{
      public:
         //process phsyics events
         inline static void process(){
            for (auto& [id, layer] : instance().collisionLayers){
               layer.checkLayer(id);
            }
         }
         inline static void addColliderToLayer(int layer, Collision::CollisionRect& collider){
            instance().collisionLayers[layer].rects.push_back(&collider);
         }
//         inline static void addColliderToLayer(int layer, Collision::CollisionCircle& collider){
////            instance().collisionLayers[layer].circles.push_back(&collider);
//         }

         inline static void removeColliderFromLayer(int layer, Collision::Collider& collider){
            bool noRun = false;
            Tools::AnonymousDtor dtor([&](){
               if (noRun) return; //nothing to delete
               std::remove(collider.layers.begin(), collider.layers.end(), layer);
            });
            auto foundLayer = instance().collisionLayers.find(layer);
            if (foundLayer == instance().collisionLayers.end()) return;
            {
               auto foundRect = std::find(foundLayer->second.rects.begin(), foundLayer->second.rects.end(), &collider);
               if (foundRect != foundLayer->second.rects.end()) {
                  foundLayer->second.rects.erase(foundRect);
                  return;
               }
            }
//            auto foundCircle = std::find(foundLayer->second.circles.begin(), foundLayer->second.circles.end(), &collider);
//            if (foundCircle != foundLayer->second.circles.end()){
//               foundLayer->second.circles.erase(foundCircle);
//               return;
//            }
            //not found in any layers
            noRun = true;
         }
         std::optional<std::reference_wrapper<Collision::CollisionLayer>> getLayer(int layer){
            auto foundLayer = collisionLayers.find(layer);
            if (foundLayer != collisionLayers.end()){
               return foundLayer->second;
            }
            return {};
         }
      private:
         static PhysicsSystem& instance();
         static std::unique_ptr<PhysicsSystem> _self;
         std::map<int, Collision::CollisionLayer> collisionLayers;
      };
   }
}
