#pragma once
#include "ReyEngine.h"

namespace ReyEngine{
   // Holds all the relevent data required for rendering
   struct RenderData2D {

   };
   // The render manager makes sure things get rendered in the right order. This way everything looks
   // like it is where it is supposed to be.
   class RenderManager2D {
      std::vector<RenderData2D> renderData;
   };
}