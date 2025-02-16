#pragma once
#include "InputManager.h"

namespace ReyEngine{
   using Handled = bool;

   namespace Internal {

      class InputHandler {
      public:
      protected:
         virtual void _on_mouse_enter(){};
         virtual void _on_mouse_exit(){};
         virtual void _on_modality_gained(){}
         virtual void _on_modality_lost(){}
         virtual void _on_focus_gained(){}
         virtual void _on_focus_lost(){}
         //input
         virtual Handled _process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) = 0; //pass input to children if they want it and then process it for ourselves if necessary
         virtual Handled __process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){ return _process_unhandled_input(event, mouse);}
         virtual Handled _process_unhandled_editor_input(const InputEvent&, const std::optional<UnhandledMouseInput>&){}; //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
         virtual UnhandledMouseInput toMouseInput(const Pos<R_FLOAT>& global) const {return {};}
         InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
      };
   }
}