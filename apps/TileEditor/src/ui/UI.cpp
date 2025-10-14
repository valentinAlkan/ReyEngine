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
   {
      auto [ptr, node] = make_node<TilePalette>("TilePalette");
      _panel->addChild(std::move(node));
   }
   _panel->setSize(100, 500);
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
   static constexpr Pos<float> CORNER_OFFSET = {-20,20};
   if (_panel){
      auto topRight = getSizeRect().topRight() + CORNER_OFFSET;
      _panel->alignTop(topRight);
      _panel->alignRight(topRight);
   }
}