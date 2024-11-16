#include "Physics.h"

using namespace std;
using namespace ReyEngine;
using namespace Physics;

std::unique_ptr<PhysicsSystem> PhysicsSystem::_self;
/////////////////////////////////////////////////////////////////////////////////////////
PhysicsSystem& PhysicsSystem::instance() {
   if (!_self) {
      _self.reset(new PhysicsSystem);
   }
   return *_self;
}
