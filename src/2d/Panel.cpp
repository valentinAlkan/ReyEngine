#include "Panel.h"
#include "Button.h"
#include "Canvas.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2D() const {
   drawRectangle(getSizeRect(), Colors::blue);
//   auto color = theme->background.colorPrimary;
//   //draw the menu bar top half that peeks out
//   auto menuBarHeight = menuBar->getHeight();
//   drawRectangle(menuBar->getRect(), theme->background.colorSecondary);
//
//
//   if (!_isMinimized) {
//      //dont need to draw these if we're minimized
//
//      static constexpr float roundness = 2.0;
//
//      //draw the rounded bottom portion
//      drawRectangle(getSizeRect().chopTop(menuBarHeight), theme->background.colorPrimary);
//      drawRectangleLines(getSizeRect().chopTop(menuBarHeight), 1.0, theme->background.colorSecondary);
//      drawRectangleRounded(getSizeRect().chopTop(menuBarHeight), roundness, 1, theme->background.colorPrimary);
//      drawRectangleRoundedLines(getSizeRect().chopBottom(menuBarHeight), roundness, 1, 1.0, Colors::black);
//
//      //draw the non-rounded band
//      drawRectangle(getSizeRect().chopBottom(50) + Pos<int>(0, menuBarHeight), theme->background.colorPrimary);
//      drawRectangleRoundedLines(getSizeRect().chopBottom(menuBarHeight), roundness, 1, 1.0, Colors::black);
//
//      //debug:
////      draw the stretch regions
//      for (const auto &region: stretchRegion) {
//         drawRectangle(region, ColorRGBA(255, 0, 0, 128));
//      }
//   }

}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2DBegin() {
//   startScissor(_scissorArea);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2DEnd() {
//   stopScissor();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_init() {
   setAcceptsHover(true);
   //create subwidgets
   vlayout = make_child<Layout>(getNode(), VLAYOUT_NAME, Layout::LayoutDir::VERTICAL);
   if (!window) window = make_child<Control>(vlayout->getNode(), WINDOW_NAME);
   menuBar = make_child<Layout>(vlayout->getNode(), VLAYOUT_NAME, Layout::LayoutDir::HORIZONTAL);
   menuBar->getTheme().layoutMargins.setAll(2);

   //get rid of control background so we only see panel background
   window->getTheme().background.fill = Style::Fill::NONE;

   vlayout->setAnchoring(Anchor::FILL);

   //cap menu bar size
   menuBar->setMaxSize({ReyEngine::MaxFloat, theme->font.size+4});
   menuBar->getTheme().background.fill = Style::Fill::SOLID;

   //add a spacer
   auto lspacer = make_child<Control>(menuBar->getNode(), "__lspacer");
   lspacer->setVisible(false);

   titleLabel = make_child<Label>(menuBar->getNode(), TITLE_LABEL_NAME, "TEXT");
   titleLabel->setTheme(theme);
   titleLabel->setText(panelTitle.empty() ? getName() : panelTitle);

   //add another spacer
   auto rspacer = make_child<Control>(menuBar->getNode(), "__rspacer");
   rspacer->setVisible(false);

   //add a button cluster on the right side of the menu bar
   auto btnClusterRight = make_child<Layout>(menuBar->getNode(), "__btnClusterRight", Layout::LayoutDir::HORIZONTAL);
   menuBar->layoutRatios = {1, 1, 1, .5};

   //add some buttons
   auto btnMin = make_child<PushButton>(btnClusterRight->getNode(), BTN_MIN_NAME); btnMin->setText("_");
   auto btnMax = make_child<PushButton>(btnClusterRight->getNode(), BTN_MAX_NAME); btnMax->setText("o");
   auto btnClose = make_child<PushButton>(btnClusterRight->getNode(), BTN_CLOSE_NAME);
   btnClose->setText("x");
   btnClusterRight->getTheme().layoutMargins.setAll(2);
   btnClusterRight->setMaxSize({100, 999999});

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
         _filterCache = window->getInputFiltering();
         window->setInputFiltering(InputFilter::IGNORE_AND_STOP);
      } else {
         window->setInputFiltering(_filterCache);
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
         cacheRect = getRect();
         if (auto parent = getParentWidget()) {
            setRect(parent.value()->getSizeRect());
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
   auto windowRectChangeCB = [&](Control& _window){
      _window.setScissorArea(_window.getSizeRect().embiggen(-1));
   };
   window->setRectChangedCallback(windowRectChangeCB);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_rect_changed(){
   //adjust stretch regions
   static constexpr int STRETCH_REGION_SIZE = 5;
   stretchRegion.at(0)= getSizeRect().chopBottom(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(1)= getSizeRect().chopLeft(getWidth() - STRETCH_REGION_SIZE);
   stretchRegion.at(2)= getSizeRect().chopTop(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(3)= getSizeRect().chopRight(getWidth() - STRETCH_REGION_SIZE);
   _scissorArea = getScissorArea();
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget *Panel::_unhandled_input(const ReyEngine::InputEvent& event) {
   auto getStretchDir = [&]() -> ResizeDir {
      auto cursorDir = ResizeDir::NONE;
      if (auto mouse = event.isMouse()) {
         //set reszie cursor
         //see if we're in a stretch region
         auto stretchDir1 = ResizeDir::NONE;
         auto stretchDir2 = ResizeDir::NONE;
         for (int i = 0; i < 4; i++) {
            //pick the appropriate stretch dir
            auto& stretchDir = stretchDir1 == ResizeDir::NONE ? stretchDir1 : stretchDir2;
            auto& region = stretchRegion.at(i);
            if (region.contains(mouse.value()->getLocalPos())) {
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
         cursorDir = stretchDir1;
         //because we go clockwise from top to left, stretchdir2 can only be a higher value than stretdir1. no need
         // to check every possible condition since there are only 4 possible states.
         if (stretchDir1 == ResizeDir::N && stretchDir2 == ResizeDir::E) cursorDir = ResizeDir::NE;
         else if (stretchDir1 == ResizeDir::E && stretchDir2 == ResizeDir::S) cursorDir = ResizeDir::SE;
         else if (stretchDir1 == ResizeDir::S && stretchDir2 == ResizeDir::W) cursorDir = ResizeDir::SW;
         else if (stretchDir1 == ResizeDir::N && stretchDir2 == ResizeDir::W) cursorDir = ResizeDir::NW;
      }
      return cursorDir;
   };

   switch (event.eventId) {
      case InputEventMouseButton::getUniqueEventId(): {
         auto &mbEvent = event.toEvent<InputEventMouseButton>();
         if (mbEvent.button != InputInterface::MouseButton::LEFT) break;
         if (mbEvent.isDown && !mbEvent.mouse.isInside()) break; //ingore downs that occur outside the rect
         dragStart = mbEvent.mouse.getLocalPos();
         resizeStartRect = getRect();
         offset = (Pos<R_FLOAT>)mousePos - getPos(); //record position

         if (!_isMinimized && _isResizable && _resizeDir == ResizeDir::NONE){
            //if we're resizing, stop here and return
            //see if we clicked in a resize region
            _resizeDir = getStretchDir();
         }

         //start dragging
         if (!_isMaximized && _resizeDir == ResizeDir::NONE && menuBar->isInside(mbEvent.mouse.getLocalPos()) && mbEvent.isDown) {
            _isDragging = true;
            return this;
         } else if ((_isDragging || _resizeDir != ResizeDir::NONE) && !mbEvent.isDown){
            //make sure to check for dragging or resizing states otherwise this will eat all mouse-ups intended for other widgets
            _isDragging = false;
            _resizeDir = ResizeDir::NONE;
            return this;
         }
      }
      break;
      case InputEventMouseMotion::getUniqueEventId():
         auto& mmEvent = event.toEvent<InputEventMouseMotion>();
         //no dragging or resizing if we're maximized
         if (_isMaximized) break;
         mousePos = mmEvent.mouse.getLocalPos();
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
            setPosition(mousePos - offset);
            return this;
         } else {
            if (!mmEvent.mouse.isInside() || _isMinimized) break;
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
//                  InputInterface::setCursor();
                  return nullptr;
            }
            return this;
         }
         break;
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::addChildToPanel(std::shared_ptr<Widget> child){
   if (!window) window = make_child<Control>(child->getNode(), WINDOW_NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////
//void Panel::registerProperties(){
   //register properties specific to your type here.
//}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<R_FLOAT> Panel::getScissorArea() {
   if (_isMinimized){
      return menuBar->getRect();
   }
   return getSizeRect();
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