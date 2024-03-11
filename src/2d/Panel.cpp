#include "Panel.h"
#include "Button.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render() const {
   auto roundness = theme->roundness.value;
   auto color = theme->background.colorPrimary;
   //draw the menu bar top half that peeks out
   auto menuBarHeight = menuBar->getHeight();
   drawRectangleRounded(_rect.value.toSizeRect().chopBottom(menuBarHeight), roundness, 1, theme->background.colorSecondary);

   //draw the rounded bottom portion
   drawRectangleRounded(_rect.value.toSizeRect().chopTop(menuBarHeight), roundness, 1, theme->background.colorPrimary);

   //draw the non-rounded band
   drawRectangle(_rect.value.toSizeRect().chopBottom(50) + Pos<int>(0,menuBarHeight), theme->background.colorPrimary);


   //draw the grab bar
   if (showHeader){
//      drawRectangle()
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_init() {
   //create subwidgets
   vlayout = make_shared<VLayout>(VLAYOUT_NAME);
   if (!window) window = make_shared<Control>(WINDOW_NAME);
   menuBar = make_shared<HLayout>(MENU_NAME);

   //get rid of control backgrounds so we only see panel background
   window->getTheme()->background = Style::Fill::NONE;

   addChild(vlayout);
   vlayout->setAnchoring(BaseWidget::Anchor::FILL);
   vlayout->addChild(menuBar);
   vlayout->addChild(window);

   //cap menu bar size
   menuBar->setMaxSize({ReyEngine::MaxInt, (int)theme->font.value.size});
   menuBar->getTheme()->background = Style::Fill::SOLID;

   //add a spacer
   auto spacer = make_shared<Control>("__spacer");
   spacer->setVisible(false);
   menuBar->addChild(spacer);

   //add a button cluster on the right side of the menu bar
   auto btnClusterRight = make_shared<HLayout>("__btnClusterRight");
   menuBar->addChild(btnClusterRight);
   menuBar->childScales = {0.95, .05};

   //add some buttons
   auto closeButton = make_shared<PushButton>(BTN_CLOSE_NAME);
   closeButton->setText("x");
//   closeButton->setMaxSize({(int)theme->font.value.size, (int)theme->font.value.size});
   btnClusterRight->addChild(closeButton);

   //connect to button close signal
   auto toggleShowCB = [this](const PushButton::ButtonPressEvent& event){
      setVisible(false);
      return true;
   };
   subscribe<PushButton::ButtonPressEvent>(closeButton, toggleShowCB);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Panel::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){
   switch (event.eventId) {
      case InputEventMouseButton::getUniqueEventId(): {
         auto &mbEvent = event.toEventType<InputEventMouseButton>();
         _isDragging = mbEvent.isDown && menuBar->isInside(mouse->localPos);
         return true;
      }
      case InputEventMouseMotion::getUniqueEventId():
         mousePos = InputManager::getMousePos();
         if (_isDragging) {
            setPos(mousePos - offset);
         } else {
            offset = mousePos - getPos();
         }

         return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> Panel::addChildToPanel(std::shared_ptr<BaseWidget> child){
   if (!window) window = make_shared<Control>(WINDOW_NAME);
   return window->addChild(child);
}
/////////////////////////////////////////////////////////////////////////////////////////
void Panel::registerProperties(){
   //register properties specific to your type here.

}