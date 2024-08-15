#pragma once
#include "BaseBody.h"

namespace ReyEngine {
   class MeshBody : public BaseBody {
      REYENGINE_OBJECT_BUILD_ONLY(MeshBody, BaseBody){}
   public:
      REYENGINE_DEFAULT_BUILD(MeshBody);
   };
}