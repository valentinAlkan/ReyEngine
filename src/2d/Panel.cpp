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

}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::render2D() const {
   //draw the header
   drawRectangle(_header.rect, theme->background.colorTertiary);
   drawRectangle(_header.btnClose, Colors::red);
   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);
   drawLine(_header.rect.bottom(), 1.0, theme->background.colorSecondary);
   drawTextCentered("x", _header.btnClose.center(), theme->font);
//   if (!_isMinimized) {
//      //dont need to draw these if we're minimized
//      static constexpr float roundness = 2.0;
//
//      //draw the rounded bottom portion
//      drawRectangle(getSizeRect(), theme->background.colorPrimary);
//
//      drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);
//
//      //debug:
////      draw the stretch regions
//      for (const auto &region: stretchRegion) {
//         drawRectangle(region, ColorRGBA(0, 0, 255, 128));
//      }
//   }

}

/////////////////////////////////////////////////////////////////////////////////////////
//void Panel::render2DBegin() {
//   startScissor(_scissorArea);
//   mask.beginRenderMode();
//   ClearBackground(Colors::black);
//   drawRectangleRounded(getSizeRect(), 5, 10, Colors::white);
//   mask.endRenderMode();
//   BeginDrawing();
//}

/////////////////////////////////////////////////////////////////////////////////////////
//void Panel::render2DEnd() {
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::__init() {
   Canvas::_init();
   theme->background.fill = Style::Fill::SOLID;
   theme->background.colorPrimary = ReyEngine::ColorRGBA(94, 142, 181, 255);
   _viewArea = ::make_child<Widget>(this, "viewArea");
   _on_rect_changed();
   setAcceptsHover(true);
   _inputFilter = InputFilter::PROCESS_AND_PASS;
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
//   menuBar->setMaxSize({ReyEngine::MaxFloat, theme->font->size+4});
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
//   menuBar.btnCluster = make_child_built_in<Layout>("__btnClusterRight", Layout::LayoutDir::HORIZONTAL);
//   menuBar.btnCluster->setMinWidth(20 * 3);
//   menuBar.btnCluster->setMinHeight(menuBar.bar.height);

   //add some buttons
//   auto btnMin = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_MIN_NAME, "_");
//   auto btnMax = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_MAX_NAME, "o");
//   auto btnClose = ReyEngine::make_child<PushButton>(menuBar.btnCluster->getNode(), BTN_CLOSE_NAME, "x");
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
   _init();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_rect_changed(){
   //update render masks
   //adjust stretch regions
   static constexpr int STRETCH_REGION_SIZE = 5;
   stretchRegion.at(0) = getSizeRect().splitAtHPos(getHeight() - STRETCH_REGION_SIZE).first;
   stretchRegion.at(1) = getSizeRect().splitAtVPos(getWidth() - STRETCH_REGION_SIZE).second;
   stretchRegion.at(2) = getSizeRect().splitAtHPos(getHeight() - STRETCH_REGION_SIZE).second;
   stretchRegion.at(3) = getSizeRect().splitAtVPos(getWidth() - STRETCH_REGION_SIZE).first;

   //update viewable area
   _viewArea->setRect(getSizeRect().splitAtVPos(_header.rect.height).second.embiggen(-theme->layoutMargin));

   //header bar
   static constexpr float HEADER_HEIGHT = 35;
   static constexpr float BUTTON_SIZE = HEADER_HEIGHT - 10;
   _header.rect = {0, 0, getWidth(), HEADER_HEIGHT};
   _header.btnClose = {{0,0}, Size<float>(BUTTON_SIZE)};
   _header.btnClose = _header.btnClose.centerV(_header.rect.right().midpoint());
   _header.btnClose = _header.btnClose.alignRight(getSizeRect().right().midpoint() - Pos<float>(_header.btnClose.topRight().y, 0));
}

/////////////////////////////////////////////////////////////////////////////////////////
TypeNode* Panel::addChild(std::unique_ptr<TypeNode>&& child) {
   return _viewArea->addChild(std::move(child));
}

///////////////////////////////////////////////////////////////////////////////////////
Widget* Panel::_unhandled_input(const ReyEngine::InputEvent& event) {
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
         if (_dragState == DragState::NONE && !mbEvent.isDown && !mbEvent.mouse.isInside()){
            hide();
            return this;
         }
         if (!mbEvent.isDown && _header.btnClose.contains(mbEvent.mouse.getLocalPos())){
            hide();
         }
         dragStart = mbEvent.mouse.getLocalPos();
//         resizeStartRect = getRect();
//         offset = mbEvent.mouse.getLocalPos() - getPos(); //record position

//         if (!_isMinimized && _isResizable && _resizeDir == ResizeDir::NONE){
//            //if we're resizing, stop here and return
//            //see if we clicked in a resize region
//            _resizeDir = getStretchDir();
//         }

         //dragging logic
         if (_header.rect.contains(mbEvent.mouse.getLocalPos()) && _isDragable) {
            if (mbEvent.isDown) {
               //start dragging
               _dragState = DragState::DRAGGING;
            } else {
               // stop dragging
               _dragState = DragState::NONE;
            }
         }
//            if (_isResizable && resizeStartRect) {
//               if (!_isMaximized && _resizeDir == ResizeDir::NONE && _header.rect.contains(mbEvent.mouse.getLocalPos()) && mbEvent.isDown) {
//                  _dragState = DragState::DRAGGING;
//                  return this;
//               } else if ((_dragState == DragState::DRAGGING || _resizeDir != ResizeDir::NONE) && !mbEvent.isDown) {
//                  //make sure to check for dragging or resizing states otherwise this will eat all mouse-ups intended for other widgets
//                  _dragState = DragState::NONE;
//                  _resizeDir = ResizeDir::NONE;
//                  return this;
//               }
//            }
      }
         break;
      case InputEventMouseMotion::getUniqueEventId():
         auto &mmEvent = event.toEvent<InputEventMouseMotion>();
         //no dragging or resizing if we're maximized
         if (_isMaximized) break;
         auto delta = mmEvent.mouse.getLocalPos() - dragStart;
         if (_dragState == DragState::DRAGGING) {
            movePosition(delta);
            return this;
         }
   }

//         //stretching overrides dragging
//         auto stretchN = [&](Rect<float> newRect){newRect.y+=delta.y; newRect.height -= delta.y; return newRect;};
//         auto stretchE = [&](Rect<float> newRect){newRect.width += delta.x; return newRect;};
//         auto stretchW = [&](Rect<float> newRect){newRect.x+=delta.x; newRect.width -= delta.x; return newRect;};
//         auto stretchS = [&](Rect<float> newRect){newRect.height += delta.y; return newRect;};

//         if (_isResizable && _resizeDir != ResizeDir::NONE) {
////            Logger::info() << "Old rect size = " << getRect() << endl;
//            switch (_resizeDir){
//               case ResizeDir::N: setRect(stretchN(resizeStartRect));break;
//               case ResizeDir::E: setRect(stretchE(resizeStartRect)); break;
//               case ResizeDir::W: setRect(stretchW(resizeStartRect)); break;
//               case ResizeDir::S: setRect(stretchS(resizeStartRect)); break;
//               case ResizeDir::NW: setRect(stretchW(stretchN(resizeStartRect))); break;
//               case ResizeDir::NE: setRect(stretchN(stretchE(resizeStartRect))); break;
//               case ResizeDir::SE: setRect(stretchS(stretchE(resizeStartRect))); break;
//               case ResizeDir::SW: setRect(stretchS(stretchW(resizeStartRect))); break;
//            }
////            Logger::info() << "New Rect size = " << getRect() << endl;
////            Logger::info() << "Current Mouse pos = " << mmEvent.mouse.getLocalPos() << endl;
////            Logger::info() << "Start rect = " << resizeStartRect << endl;
////            Logger::info() << "Delta = " << delta << endl;
////            Logger::info() << "-----------------" << endl;
//            return this;
//         } else if (_isDragging) {
//            movePosition(mmEvent.mouse.getLocalPos() - dragStart);
//            return this;
//         } else {
//            if (!_isResizable) break;
//            if (!mmEvent.mouse.isInside() || _isMinimized) break;
//            //update cursor
//            auto cursorDir = getStretchDir();
//            switch (cursorDir) {
//               case ResizeDir::N:
//               case ResizeDir::S:
//                  InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NS);
//                  break;
//               case ResizeDir::E:
//               case ResizeDir::W:
//                  InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_EW);
//                  break;
//               case ResizeDir::NW:
//               case ResizeDir::SE:
//                  InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NWSE);
//                  break;
//               case ResizeDir::NE:
//               case ResizeDir::SW:
//                  InputInterface::setCursor(InputInterface::MouseCursor::RESIZE_NESW);
//                  break;
//               default:
//                  InputInterface::setCursor(InputInterface::MouseCursor::DEFAULT);
//                  break;
//            }
//            return this;
//         }
//   }

   //consume all mouse input that is inside the panel so it doesn't fall through to the area behind
//   if (auto mouse = event.isMouse()){
//      if (getSizeRect().contains(mouse.value()->getLocalPos())) {
//         Logger::info() << "Panel consuming event!" << endl;
//         return this;
//      }
//   }
   //consume all inside mouse events
   if (auto isMouse = event.isMouse()){
      if (isMouse.value()->isInside()) return this;
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
//void Panel::registerProperties(){
   //register properties specific to your type here.
//}

/////////////////////////////////////////////////////////////////////////////////////////
//ReyEngine::Rect<R_FLOAT> Panel::getScissorArea() {
//   if (_isMinimized){
//      return _menuBar;
//   }
//   return getSizeRect();
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::_on_mouse_exit() {
//   InputInterface::setCursor(InputInterface::MouseCursor::DEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::setTitle(const std::string &newtitle) {
   panelTitle = newtitle;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::hide(){
   setVisible(false);
   _dragState = DragState::NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Panel::show(){
   setVisible(true);
}