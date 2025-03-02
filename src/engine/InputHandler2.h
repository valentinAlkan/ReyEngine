#pragma once
#include "InputManager2.h"

namespace ReyEngine{
   using Handled = bool;
   class Window;
   class Canvas;
   namespace Internal {

      class InputHandler : public TypeTag {
      public:
      protected:
         virtual void _on_mouse_enter(){};
         virtual void _on_mouse_exit(){};
         virtual void _on_modality_gained(){}
         virtual void _on_modality_lost(){}
         virtual void _on_focus_gained(){}
         virtual void _on_focus_lost(){}
         //input
         virtual Handled _unhandled_input(const InputEvent&) = 0; //pass input to children if they want it and then process it for ourselves if necessary
         virtual Handled __process_unhandled_input(const InputEvent& event){ return _unhandled_input(event);}
         virtual Handled _process_unhandled_editor_input(const InputEvent&){return false;} //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
         InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
         friend class ReyEngine::Canvas;
      };
   }
}