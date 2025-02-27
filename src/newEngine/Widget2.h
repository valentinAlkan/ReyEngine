#pragma once
#include "Drawable2D.h"
#include "InputHandler2.h"

namespace ReyEngine {
   class Widget2
   : public Internal::Drawable2D
   , public Internal::InputHandler
   {
   public:
      REYENGINE_OBJECT(Canvas)
   protected:
      Handled _process_unhandled_input(const InputEvent& event) override {return false;}
   };
}