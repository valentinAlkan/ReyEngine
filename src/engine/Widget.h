#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"

namespace ReyEngine {
   class Widget
   : public Internal::Drawable2D
   , public Internal::InputHandler
   {
   public:
      REYENGINE_OBJECT(Canvas)
   protected:
      Handled _unhandled_input(const InputEvent& event) override {return false;}
   };
}