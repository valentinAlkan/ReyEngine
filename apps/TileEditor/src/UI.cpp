#include "UI.h"
#include "Label.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
UI::UI(std::shared_ptr<TileEditor>& editor)
: _editor(editor)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void UI::_init() {
   {
      auto [label, node] = make_node<Label>("UILabel", "Hello from UI");
      addChild(std::move(node));
   }
   _editor->setAnchoring(ReyEngine::Anchor::FILL);
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* UI::_unhandled_input(const InputEvent& event){
   switch (event.eventId){
      case InputEventMouseWheel::ID:
         const auto& mwEvent = event.toEvent<InputEventMouseWheel>();
         auto& camera = getCanvas().value()->getCamera();
         const float zoomAmt = camera.zoom * .1;
         mwEvent.wheelMove.y > 0 ? camera.zoom += zoomAmt : camera.zoom -= zoomAmt;
         camera.target
         return this;
   }
   return nullptr;
}