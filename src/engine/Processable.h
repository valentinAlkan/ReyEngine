#pragma once

namespace ReyEngine::Internal{
   struct Processable{
      virtual void _process(float dt) = 0;
   };

}