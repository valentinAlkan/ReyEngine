#pragma once
#include "BaseBody.h"

namespace ReyEngine {
   class Plane3D : public BaseBody {
      REYENGINE_OBJECT_BUILD_ONLY(Plane3D, BaseBody, BaseBody) {}
   public:
      REYENGINE_DEFAULT_BUILD(Plane3D)
   };
}
