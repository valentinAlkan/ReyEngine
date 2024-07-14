#include "Panel.h"
#include "Button.h"
#include "Canvas.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render() const {
   auto color = theme->background.colorPrimary;
   //draw the menu bar top half that peeks out
   auto menuBarHeight = menuBar->getHeight();
   drawRectangle(menuBar->getRect(), theme->background.colorSecondary);


   if (!_isMinimized) {
      //dont need to draw these if we're minimized

      //draw the rounded bottom portion
      drawRectangle(_rect.value.toSizeRect().chopTop(menuBarHeight), theme->background.colorPrimary);
      drawRectangleLines(_rect.value.toSizeRect().chopTop(menuBarHeight), 1.0, theme->background.colorSecondary);
//      drawRectangleRounded(_rect.value.toSizeRect().chopTop(menuBarHeight), roundness, 1, theme->background.colorPrimary);
//      drawRectangleRoundedLines(_rect.value.toSizeRect().chopBottom(menuBarHeight), roundness, 1, 1.0, Colors::black);

      //draw the non-rounded band
//      drawRectangle(_rect.value.toSizeRect().chopBottom(50) + Pos<int>(0, menuBarHeight), theme->background.colorPrimary);
//      drawRectangleRoundedLines(_rect.value.toSizeRect().chopBottom(menuBarHeight), roundness, 1, 1.0, Colors::black);

      //debug:
      //draw the stretch regions
//      for (const auto &region: stretchRegion) {
//         drawRectangle(region, ColorRGBA(255, 0, 0, 128));
//      }
   }

}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::renderBegin(ReyEngine::Pos<double> &textureOffset) {
   startScissor(_scissorArea);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::renderEnd() {
   stopScissor();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_init() {
   setAcceptsHover(true);
   //create subwidgets
   vlayout = VLayout::build<VLayout>(VLAYOUT_NAME);
   if (!window) window = Control::build<Control>(WINDOW_NAME);
   menuBar = HLayout::build<HLayout>(MENU_NAME);
   menuBar->getTheme()->layoutMargins.setAll(2);

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
   auto lspacer = Control::build<Control>("__lspacer");
   lspacer->setVisible(false);
   menuBar->addChild(lspacer);

   titleLabel = Label::build<Label>(TITLE_LABEL_NAME);
   titleLabel->setTheme(theme);
   menuBar->addChild(titleLabel);
   titleLabel->setText(panelTitle.value.empty() ? getName() : panelTitle.value);

   //add another spacer
   auto rspacer = Control::build<Control>("__rspacer");
   rspacer->setVisible(false);
   menuBar->addChild(rspacer);

   //add a button cluster on the right side of the menu bar
   auto btnClusterRight = HLayout::build<HLayout>("__btnClusterRight");
   menuBar->addChild(btnClusterRight);
   menuBar->childScales = {1,1,1,.5};

   //add some buttons
   auto btnMin = PushButton::build<PushButton>(BTN_MIN_NAME); btnMin->setText("_");
   auto btnMax = PushButton::build<PushButton>(BTN_MAX_NAME); btnMax->setText("o");
   auto btnClose = PushButton::build<PushButton>(BTN_CLOSE_NAME);
   btnClose->setText("x");
   btnClusterRight->addChild(btnMin);
   btnClusterRight->addChild(btnMax);
   btnClusterRight->addChild(btnClose);
   btnClusterRight->getTheme()->layoutMargins.setAll(2);
   btnClusterRight->setMaxSize({100, 999999});
//   btnClusterRight->setMinSize({100, 999999});

   //connect to button signals
   auto setScissor = [this](){_scissorArea = getScissorArea();};
   auto toggleShowCB = [this](const PushButton::ButtonPressEvent& event){setVisible(false); return true;};
   auto toggleMinCB = [this, setScissor](const PushButton::ButtonPressEvent& event){
      _isMinimized = !_isMinimized;
      setScissor();
      window->setVisible(!_isMinimized);
      _isResizable = !_isMinimized;
      if (_isMinimized){
         //record the input filter type so we can recall it later
         _filterCache = window->getInputFilter();
         window->setInputFilter(InputFilter::INPUT_FILTER_IGNORE_AND_STOP);
      } else {
         window->setInputFilter(_filterCache);
      }
      setAcceptsHover(!_isMinimized);

      return true;
   };
   auto toggleMaxCB = [this, toggleMinCB](const PushButton::ButtonPressEvent& event){
      if (_isMinimized){
         //deminimize
         return toggleMinCB(event);
      }

      _isMaximized = !_isMaximized;
      if (_isMaximized){
         //maximize
         cacheRect = _rect;
         auto parent = getParent().lock();
         if (parent) {
            setRect({{0, 0}, parent->getSize()});
         }
      } else {
         //demaximize
         setRect(cacheRect);
      }
      return true;
   };
   subscribe<PushButton::ButtonPressEvent>(btnClose, toggleShowCB);
   subscribe<PushButton::ButtonPressEvent>(btnMin, toggleMinCB);
   subscribe<PushButton::ButtonPressEvent>(btnMax, toggleMaxCB);

   //window rect change callback
   auto windowRectChangeCB = [&](Control& window){
      window.setScissorArea(window.getRect().toSizeRect().embiggen(-1));
   };
   window->setRectChangedCallback(windowRectChangeCB);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_rect_changed(){
   //adjust stretch regions
   static constexpr int STRETCH_REGION_SIZE = 5;
   stretchRegion.at(0) = _rect.value.toSizeRect().chopBottom(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(1) = _rect.value.toSizeRect().chopLeft(getWidth() - STRETCH_REGION_SIZE);
   stretchRegion.at(2) = _rect.value.toSizeRect().chopTop(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(3) = _rect.value.toSizeRect().chopRight(getWidth() - STRETCH_REGION_SIZE);
   _scissorArea = getScissorArea();
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
         if (mbEvent.button != InputInterface::MouseButton::LEFT) break;
         if (mbEvent.isDown && !mouse->isInside) break; //ingore downs that occur outside the rect
         dragStart = InputManager::getMousePos();
         resizeStartRect = _rect.value;
         offset = mousePos - getPos(); //record position

         if (!_isMinimized && _isResizable && _resizeDir == ResizeDir::NONE){
            //if we're resizing, stop here and return
            //see if we clicked in a resize region
            _resizeDir = getStretchDir();
         }

         //start dragging
         if (!_isMaximized && _resizeDir == ResizeDir::NONE && menuBar->isInside(mouse->localPos) && mbEvent.isDown) {
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
         //no dragging or resizing if we're maximized
         if (_isMaximized) break;
         mousePos = InputManager::getMousePos();
         auto delta = mousePos - dragStart;
         //stretching overrides dragging
         auto stretchN = [&](Rect<int> newRect){newRect.y+=delta.y; newRect.height -= delta.y; return newRect;};
         auto stretchE = [&](Rect<int> newRect){newRect.width += delta.x; return newRect;};
         auto stretchW = [&](Rect<int> newRect){newRect.x+=delta.x; newRect.width -= delta.x; return newRect;};
         auto stretchS = [&](Rect<int> newRect){newRect.height += delta.y; return newRect;};

         if (_resizeDir != ResizeDir::NONE) {
            switch (_resizeDir){
               case ResizeDir::N: setRect(stretchN(resizeStartRect));break;
               case ResizeDir::E: setRect(stretchE(resizeStartRect)); break;
               case ResizeDir::W: setRect(stretchW(resizeStartRect)); break;
               case ResizeDir::S: setRect(stretchS(resizeStartRect)); break;
               case ResizeDir::NW: setRect(stretchW(stretchN(resizeStartRect))); break;
               case ResizeDir::NE: setRect(stretchN(stretchE(resizeStartRect))); break;
               case ResizeDir::SE: setRect(stretchS(stretchE(resizeStartRect))); break;
               case ResizeDir::SW: setRect(stretchS(stretchW(resizeStartRect))); break;
            }
         } else if (_isDragging) {
            setPos(mousePos - offset);
            return true;
         } else {
            if (!mouse->isInside || _isMinimized) break;
            //update cursor
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
                  return false;
            }
            return true;
         }
         break;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::addChildToPanel(std::shared_ptr<BaseWidget> child){
   if (!window) window = Control::build<Control>(WINDOW_NAME);
   window->addChild(child);
}
/////////////////////////////////////////////////////////////////////////////////////////
void Panel::registerProperties(){
   //register properties specific to your type here.
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<int> Panel::getScissorArea() {
   if (_isMinimized){
      return menuBar->getRect();
   }
   return getRect().toSizeRect();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_mouse_exit() {
   InputInterface::setCursor(InputInterface::MouseCursor::DEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::setTitle(const std::string &newtitle) {
   panelTitle = newtitle;
   if (titleLabel) {
      titleLabel->setText(newtitle);
   }
}