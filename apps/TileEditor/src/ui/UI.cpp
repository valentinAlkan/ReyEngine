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

   {
      auto [palette, node] = make_node<TilePalette>("TilePalette");
      addChild(std::move(node));
      _palette = palette;
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
         camera.target = event.toEvent<InputEventMouseWheel>().mouse.getCanvasPos().get();
         camera.offset = event.toEvent<InputEventMouseWheel>().mouse.getCanvasPos().get();
         return this;
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void UI::_on_rect_changed() {
   _palette->setPosition(getSizeRect().topRight() + Pos<float>(-_palette->getWidth() - 30, 30));
}