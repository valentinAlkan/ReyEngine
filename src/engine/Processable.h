#pragma once

namespace ReyEngine{
   class Window;
   namespace Internal{

      struct Processable{
         virtual void _process(float dt){};
         void setProcess(bool);
         [[nodiscard]] bool isProcessed() const;
      };


   }
}