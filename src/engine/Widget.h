#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"
#include "Processable.h"
#include "Theme.h"

namespace ReyEngine {
   class Widget
   : public Internal::Drawable2D
   , public Internal::InputHandler
   , public Internal::Processable
   {
   public:
      REYENGINE_OBJECT(Canvas)
      Theme& getTheme(){return *theme;}
   protected:
      Handled _unhandled_input(const InputEvent& event) override {return false;}
      void _process(float dt) override {};

      bool acceptsHover = false;
      RefCounted<Theme> theme;
   };
}