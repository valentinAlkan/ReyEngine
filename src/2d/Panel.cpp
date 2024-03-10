#include "Panel.h"
#include "Button.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render() const {
   auto roundness = theme->roundness.value;
   auto color = theme->background.colorPrimary.value;
   drawRectangleRounded(_rect.value.toSizeRect(), roundness, 1, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_init() {
   //create subwidgets
   vlayout = make_shared<VLayout>(VLAYOUT_NAME);
   window = make_shared<Control>(WINDOW_NAME);
   menuBar = make_shared<HLayout>(MENU_NAME);

   //get rid of control backgrounds so we only see panel background
   window->getTheme()->background = Style::Fill::NONE;

   addChild(vlayout);
   vlayout->setAnchoring(BaseWidget::Anchor::FILL);
   vlayout->addChild(menuBar);
   vlayout->addChild(window);

   //cap menu bar size
   menuBar->setMaxSize({ReyEngine::MaxInt, theme->menuHeight.value});
   menuBar->getTheme()->background = Style::Fill::SOLID;

   //add a button cluster on the right side of the menu bar
   auto btnClusterRight = make_shared<HLayout>("__btnClusterRight");
   menuBar->addChild(btnClusterRight);
   btnClusterRight->setAnchoring(BaseWidget::Anchor::RIGHT);

   //debug - visualize menu bar
   auto debugView = make_shared<Control>("debug");
   addChild(debugView);
   auto resizeCB = [debugView](const WidgetResizeEvent& event){
      debugView->setRect(event.publisher->toBaseWidget()->getRect());
   };

   //add some buttons
   auto closeButton = make_shared<PushButton>(BTN_CLOSE_NAME);
   menuBar->addChild(closeButton);

   //connect to button close signal
   auto toggleShowCB = [this](const PushButton::ButtonPressEvent& event){
      setVisible(false);
      return true;
   };
   subscribe<PushButton::ButtonPressEvent>(closeButton, toggleShowCB);

   debugView->subscribe<BaseWidget::WidgetResizeEvent>(menuBar, resizeCB);

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
void Panel::registerProperties(){
   //register properties specific to your type here.

}