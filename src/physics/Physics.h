#pragma once
#include <memory>
#include "CollisionShape.h"

namespace ReyEngine {
   namespace Physics {
      class PhysicsSystem{
      public:
         //process phsyics events
         inline static void process(){
            for (const auto& layer : instance().collisionLayers){
               layer.second.checkLayer();
            }
         }
      private:
         static PhysicsSystem& instance();
         static std::unique_ptr<PhysicsSystem> _self;
         std::map<int, Collision::CollisionLayer> collisionLayers;
      };
   }
}
