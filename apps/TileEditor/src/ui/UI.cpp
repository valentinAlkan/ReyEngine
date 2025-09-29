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
   auto label = make_child<Label>(this, "UILabel", "Hello from UI");
   _panel = make_child<Panel>(this, "_panel");
//   _palette = _panel->make_child<TilePalette>("TilePalette");
   _editor->setAnchoring(ReyEngine::Anchor::FILL);
   _panel->addChildToPanel(_palette);
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
   if (_palette) _palette->setPosition(getSizeRect().topRight() + Pos<float>(-_palette->getWidth() - 30, 30));
}