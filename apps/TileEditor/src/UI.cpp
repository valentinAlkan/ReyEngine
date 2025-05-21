#include "UI.h"
#include "Label.h"
#include "Canvas.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
UI::UI(){

}

/////////////////////////////////////////////////////////////////////////////////////////
void UI::_init() {
   {
      auto [editor, node] = make_node<TileEditor>("Editor");
      _editor = editor;
      addChild(std::move(node));
   }

   {
      auto [label, node] = make_node<Label>("UILabel", "Hello from UI");
      addChild(std::move(node));
      if (auto canvas = getCanvas()){
         canvas.value()->moveToForeground(label.get());
      }

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
         return this;
   }
   return nullptr;
}