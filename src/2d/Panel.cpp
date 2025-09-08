#include "Panel.h"
#include "Button.h"
#include "Canvas.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
Panel::Panel()
: REGION_NORTH(stretchRegion[0])
, REGION_EAST(stretchRegion[1])
, REGION_SOUTH(stretchRegion[2])
, REGION_WEST(stretchRegion[3])
{
   theme->background.fill = Style::Fill::SOLID;
   theme->background.colorPrimary = ReyEngine::ColorRGBA(94, 142, 181, 255);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2D() const {
   auto color = theme->background.colorPrimary;
   //draw the menu bar top half that peeks out
   auto menuBarHeight = menuBar.bar.height;
   drawRectangleLines(menuBar.bar, 1.0, theme->background.colorSecondary);
   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);

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
      for (const auto &region: stretchRegion) {
         drawRectangle(region, ColorRGBA(0, 0, 255, 128));
      }
//   }

}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2DBegin() {
//   startScissor(_scissorArea);
//   mask.beginRenderMode();
//   ClearBackground(Colors::black);
//   drawRectangleRounded(getSizeRect(), 5, 10, Colors::white);
//   mask.endRenderMode();
//   BeginDrawing();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2DEnd() {
//   stopScissor();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_init() {
   _viewArea = make_child<Control>("__placementWidget");
   setAcceptsHover(true);
   //create subwidgets
//   vlayout = make_child<Layout>(getNode(), VLAYOUT_NAME, Layout::LayoutDir::VERTICAL);
//   if (!window) window = make_child<Control>(vlayout->getNode(), WINDOW_NAME);
//   menuBar = make_child<Layout>(vlayout->getNode(), VLAYOUT_NAME, Layout::LayoutDir::HORIZONTAL);
//   menuBar->getTheme().layoutMargins.setAll(2);

   //get rid of control background so we only see panel background
//   window->getTheme().background.fill = Style::Fill::NONE;

//   vlayout->setAnchoring(Anchor::FILL);
//
//   //cap menu bar size
//   menuBar->setMaxSize({ReyEngine::MaxFloat, theme->font.size+4});
//   menuBar->getTheme().background.fill = Style::Fill::SOLID;
//
//   //add a spacer
//   auto lspacer = make_child<Control>(menuBar->getNode(), "__lspacer");
//   lspacer->setVisible(false);
//
//   titleLabel = make_child<Label>(menuBar->getNode(), TITLE_LABEL_NAME, "TEXT");
//   titleLabel->setTheme(theme);
//   titleLabel->setText(panelTitle.empty() ? getName() : panelTitle);
//
//   //add another spacer
//   auto rspacer = make_child<Control>(menuBar->getNode(), "__rspacer");
//   rspacer->setVisible(false);
//
   //add a button cluster on the right side of the menu bar
   menuBar.btnCluster = make_child_built_in<Layout>("__btnClusterRight", Layout::LayoutDir::HORIZONTAL);
   menuBar.btnCluster->setMinWidth(20 * 3);
   menuBar.btnCluster->setMinHeight(menuBar.bar.height);

   //add some buttons
   auto btnMin = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_MIN_NAME, "_");
   auto btnMax = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_MAX_NAME, "o");
   auto btnClose = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_CLOSE_NAME, "x");
//   btnClusterRight->getTheme().layoutMargins.setAll(2);
//   btnClusterRight->setMaxSize({100, 999999});
//
//   //connect to button signals
//   auto setScissor = [this](){_scissorArea = getScissorArea();};
//   auto toggleShowCB = [this](const PushButton::ButtonPressEvent& event){setVisible(false); return true;};
//   auto toggleMinCB = [this, setScissor](const PushButton::ButtonPressEvent& event){
//      _isMinimized = !_isMinimized;
//      setScissor();
//      window->setVisible(!_isMinimized);
//      _isResizable = !_isMinimized;
//      if (_isMinimized){
//         //record the input filter type so we can recall it later
//         _filterCache = window->getInputFiltering();
//         window->setInputFiltering(InputFilter::IGNORE_AND_STOP);
//      } else {
//         window->setInputFiltering(_filterCache);
//      }
//      setAcceptsHover(!_isMinimized);
//
//      return true;
//   };
//   auto toggleMaxCB = [this, toggleMinCB](const PushButton::ButtonPressEvent& event){
//      if (_isMinimized){
//         //deminimize
//         return toggleMinCB(event);
//      }
//
//      _isMaximized = !_isMaximized;
//      if (_isMaximized){
//         //maximize
//         cacheRect = getRect();
//         if (auto parent = getParentWidget()) {
//            setRect(parent.value()->getSizeRect());
//         }
//      } else {
//         //demaximize
//         setRect(cacheRect);
//      }
//      return true;
//   };
//   subscribe<PushButton::ButtonPressEvent>(btnClose, toggleShowCB);
//   subscribe<PushButton::ButtonPressEvent>(btnMin, toggleMinCB);
//   subscribe<PushButton::ButtonPressEvent>(btnMax, toggleMaxCB);
//
//   //window rect change callback
//   auto windowRectChangeCB = [&](Control& _window){
//      _window.setScissorArea(_window.getSizeRect().embiggen(-1));
//   };
//   window->setRectChangedCallback(windowRectChangeCB);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_rect_changed(){
   //update render masks

   //adjust stretch regions
   static constexpr int STRETCH_REGION_SIZE = 5;
   stretchRegion.at(0)= getSizeRect().chopBottom(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(1)= getSizeRect().chopLeft(getWidth() - STRETCH_REGION_SIZE);
   stretchRegion.at(2)= getSizeRect().chopTop(getHeight() - STRETCH_REGION_SIZE);
   stretchRegion.at(3)= getSizeRect().chopRight(getWidth() - STRETCH_REGION_SIZE);
   _scissorArea = getScissorArea();

   //update viewable area
   _viewArea->setRect(window);

   //create rects
   static constexpr float MENU_BAR_HEIGHT = 35;
   static constexpr float BUTTON_SIZE = MENU_BAR_HEIGHT - 4;
   menuBar.bar = Rect<float>(0,0,getWidth(), MENU_BAR_HEIGHT);
   auto newClusterPos = getSizeRect().topRight();
   newClusterPos.x -= menuBar.btnCluster->getWidth();
   menuBar.btnCluster->setPosition(newClusterPos);
}

/////////////////////////////////////////////////////////////////////////////////////////
TypeNode *Panel::addChild(std::unique_ptr<TypeNode> &&child) {
   auto thiz = dynamic_cast<TypeNode*>(this);
   if (!thiz) throw std::runtime_error("Not sure how you managed to do this");
   return thiz->addChild(std::move(child));
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget *Panel::_unhandled_input(const ReyEngine::InputEvent& event) {
   auto getStretchDir = [&]() -> ResizeDir {
      if (auto mouse = event.isMouse()) {
         auto localPos = mouse.value()->getLocalPos();
         if (REGION_NORTH.contains(localPos)) {
            if (REGION_EAST.contains(localPos)) return ResizeDir::NE;
            if (REGION_WEST.contains(localPos)) return ResizeDir::NW;
            return ResizeDir::N;
         } else if (REGION_SOUTH.contains(localPos)) {
            if (REGION_EAST.contains(localPos)) return ResizeDir::SE;
            if (REGION_WEST.contains(localPos)) return ResizeDir::SW;
            return ResizeDir::S;
         }
         if (REGION_EAST.contains(localPos)) return ResizeDir::E;
         if (REGION_WEST.contains(localPos)) return ResizeDir::W;
      }
      return ResizeDir::NONE;
   };

   switch (event.eventId) {
      case InputEventMouseButton::getUniqueEventId(): {
         auto &mbEvent = event.toEvent<InputEventMouseButton>();
         if (mbEvent.button != InputInterface::MouseButton::LEFT) break;
         if (mbEvent.isDown && !mbEvent.mouse.isInside()) break; //ingore downs that occur outside the rect
         dragStart = mbEvent.mouse.getLocalPos();
         resizeStartRect = getRect();
         offset = mbEvent.mouse.getLocalPos() - getPos(); //record position

         if (!_isMinimized && _isResizable && _resizeDir == ResizeDir::NONE){
            //if we're resizing, stop here and return
            //see if we clicked in a resize region
            _resizeDir = getStretchDir();
         }

         //start dragging
         if (!_isMaximized && _resizeDir == ResizeDir::NONE && menuBar.bar.contains(mbEvent.mouse.getLocalPos()) && mbEvent.isDown) {
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
         auto delta = mmEvent.mouse.getLocalPos() - dragStart;
         //stretching overrides dragging
         Logger::info() << "Delta = " << delta << endl;
         auto stretchN = [&](){setRec.y+=delta.y; newRect.height -= delta.y; return newRect;};
         auto stretchE = [&](){newRect.width += delta.x; return newRect;};
         auto stretchW = [&](){newRect.x+=delta.x; newRect.width -= delta.x; return newRect;};
         auto stretchS = [&](){newRect.height += delta.y; return newRect;};

         if (_resizeDir != ResizeDir::NONE) {
//            Logger::info() << "Old rect size = " << getRect() << endl;
            switch (_resizeDir){
               case ResizeDir::N: stretchN();
               case ResizeDir::E: setRect(stretchE(resizeStartRect)); break;
               case ResizeDir::W: setRect(stretchW(resizeStartRect)); break;
               case ResizeDir::S: setRect(stretchS(resizeStartRect)); break;
               case ResizeDir::NW: setRect(stretchW(stretchN(resizeStartRect))); break;
               case ResizeDir::NE: setRect(stretchN(stretchE(resizeStartRect))); break;
               case ResizeDir::SE: setRect(stretchS(stretchE(resizeStartRect))); break;
               case ResizeDir::SW: setRect(stretchS(stretchW(resizeStartRect))); break;
            }
//            Logger::info() << "New Rect size = " << getRect() << endl;
//            Logger::info() << "Current Mouse pos = " << mmEvent.mouse.getLocalPos() << endl;
//            Logger::info() << "Start rect = " << resizeStartRect << endl;
//            Logger::info() << "Delta = " << delta << endl;
//            Logger::info() << "-----------------" << endl;
            return this;
         } else if (_isDragging) {
            movePosition(mmEvent.mouse.getLocalPos() - dragStart);
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
                  InputInterface::setCursor(InputInterface::MouseCursor::DEFAULT);
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
//   if (!window) window = make_child<Control>(child->getNode(), WINDOW_NAME);
}
/////////////////////////////////////////////////////////////////////////////////////////
//void Panel::registerProperties(){
   //register properties specific to your type here.
//}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<R_FLOAT> Panel::getScissorArea() {
   if (_isMinimized){
      return menuBar.bar;
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
}