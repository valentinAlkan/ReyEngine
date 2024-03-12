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

   //debug:
   //draw the stretch regions
   for (const auto& region : stretchRegion){
      drawRectangle(region, ColorRGBA(255,0,0,128));
   }

}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::renderBegin(ReyEngine::Pos<double> &textureOffset) {
   _scissorTarget.start(getGlobalRect());
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::renderEnd() {
   _scissorTarget.stop();
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
   menuBar->setMaxSize({ReyEngine::MaxInt, (int)theme->font.value.size+4});
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
void Panel::_on_rect_changed(){
   //adjust stretch regions
   static constexpr int STRETCH_REGION_SIZE = 5;
   stretchRegion.at(0) = _rect.value.toSizeRect().chopBottom(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(1) = _rect.value.toSizeRect().chopLeft(getWidth() - STRETCH_REGION_SIZE);
   stretchRegion.at(2) = _rect.value.toSizeRect().chopTop(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(3) = _rect.value.toSizeRect().chopRight(getWidth() - STRETCH_REGION_SIZE);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Panel::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){

   auto getStretchDir = [&]() {
      //set reszie cursor
      //see if we're in a stretch region
      auto stretchDir1 = ResizeDir::NONE;
      auto stretchDir2 = ResizeDir::NONE;
      for (int i = 0; i < 4; i++) {
         //pick the appropriate stretch dir
         auto &stretchDir = stretchDir1 == ResizeDir::NONE ? stretchDir1 : stretchDir2;
         auto &region = stretchRegion.at(i);
         if (region.isInside(mouse->localPos)) {
            //we're in a region. set the direction
            switch (i) {
               case 0:
                  stretchDir = ResizeDir::N;
                  break;
               case 1:
                  stretchDir = ResizeDir::E;
                  break;
               case 2:
                  stretchDir = ResizeDir::S;
                  break;
               case 3:
                  stretchDir = ResizeDir::W;
                  break;
            }
         }
         //early return - two stretch regions found
         if (stretchDir1 != ResizeDir::NONE && stretchDir2 != ResizeDir::NONE) break;
      }
      //set the resize direction
      auto cursorDir = stretchDir1;
      //because we go clockwise from top to left, stretchdir2 can only be a higher value than stretdir1. no need
      // to check every possible condition since there are only 4 possible states.
      if (stretchDir1 == ResizeDir::N && stretchDir2 == ResizeDir::E) cursorDir = ResizeDir::NE;
      else if (stretchDir1 == ResizeDir::E && stretchDir2 == ResizeDir::S) cursorDir = ResizeDir::SE;
      else if (stretchDir1 == ResizeDir::S && stretchDir2 == ResizeDir::W) cursorDir = ResizeDir::SW;
      else if (stretchDir1 == ResizeDir::N && stretchDir2 == ResizeDir::W) cursorDir = ResizeDir::NW;
      return cursorDir;
   };

   switch (event.eventId) {
      case InputEventMouseButton::getUniqueEventId(): {
         auto &mbEvent = event.toEventType<InputEventMouseButton>();
         if (mbEvent.button != InputInterface::MouseButton::LEFT) return false;
         if (mbEvent.isDown && !mouse->isInside) return false; //ingore downs that occur outside the rect
         dragStart = InputManager::getMousePos();
         startRect = _rect.value;
         offset = mousePos - getPos(); //record position

         if (_isResizable && _resizeDir == ResizeDir::NONE){
            //if we're resizing, stop here and return
            //see if we clicked in a resize region
            _resizeDir = getStretchDir();
         }

         if (_resizeDir == ResizeDir::NONE && menuBar->isInside(mouse->localPos) && mbEvent.isDown) {
            _isDragging = true;
            return true;
         } else if ((_isDragging || _resizeDir != ResizeDir::NONE) && !mbEvent.isDown){
            //make sure to check for dragging or resizing states otherwise this will eat all mouse-ups intended for other widgets
            _isDragging = false;
            _resizeDir = ResizeDir::NONE;
            return true;
         }
      }
      break;
      case InputEventMouseMotion::getUniqueEventId():
         mousePos = InputManager::getMousePos();
         auto delta = mousePos - dragStart;
         //stretching overrides dragging
         auto stretchN = [&](Rect<int> newRect){newRect.y+=delta.y; newRect.height -= delta.y; return newRect;};
         auto stretchE = [&](Rect<int> newRect){newRect.width += delta.x; return newRect;};
         auto stretchW = [&](Rect<int> newRect){newRect.x+=delta.x; newRect.width -= delta.x; return newRect;};
         auto stretchS = [&](Rect<int> newRect){newRect.height += delta.y; return newRect;};

         if (_resizeDir != ResizeDir::NONE) {
            switch (_resizeDir){
               case ResizeDir::N: setRect(stretchN(startRect));break;
               case ResizeDir::E: setRect(stretchE(startRect)); break;
               case ResizeDir::W: setRect(stretchW(startRect)); break;
               case ResizeDir::S: setRect(stretchS(startRect)); break;
               case ResizeDir::NW: setRect(stretchW(stretchN(startRect))); break;
               case ResizeDir::NE: setRect(stretchN(stretchE(startRect))); break;
               case ResizeDir::SE: setRect(stretchS(stretchE(startRect))); break;
               case ResizeDir::SW: setRect(stretchS(stretchW(startRect))); break;
            }
         } else if (_isDragging) {
            setPos(mousePos - offset);
            return true;
         } else {
            //update cursor
//            if (mouse->isInside) {
               auto cursorDir = getStretchDir();
               switch (cursorDir) {
                  case ResizeDir::N:
                  case ResizeDir::S:
                     InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NS);
                     break;
                  case ResizeDir::E:
                  case ResizeDir::W:
                     InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_EW);
                     break;
                  case ResizeDir::NW:
                  case ResizeDir::SE:
                     InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NWSE);
                     break;
                  case ResizeDir::NE:
                  case ResizeDir::SW:
                     InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NESW);
                     break;
                  default:
                     InputInterface::setCursor(cursor);
               }
//            }
            return true;
         }
         break;
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