#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"
#include <iostream>
#include <utility>
#include "Canvas.h"
#include "rlgl.h"
#include "raymath.h"

using namespace std;
using namespace ReyEngine;
using namespace FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
BaseWidget::BaseWidget(const std::string& name, std::string typeName)
: NamedInstance(name, typeName)
, Internal::Renderable2D(name, typeName)
, Component(name, typeName)
, Internal::TypeContainer<BaseWidget>(name, typeName)
, PROPERTY_DECLARE(isBackRender, false)
, PROPERTY_DECLARE(_anchor, Anchor::NONE)
, PROPERTY_DECLARE(_inputMask, InputMask::NONE)
, PROPERTY_DECLARE(enabled, false)
{}

BaseWidget::~BaseWidget() {
   Logger::debug() << "Deleting widget " << getScenePath() << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setGlobalPos(const Vec2<R_FLOAT>& newPos) {
   auto newLocalPos = globalToLocal(newPos);
   setPos(newLocalPos);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<R_FLOAT> BaseWidget::getClampedSize(ReyEngine::Size<R_FLOAT> size){
   auto newX = ReyEngine::Vec2<R_FLOAT>(minSize.x, maxSize.x).clamp(size.x);
   auto newY = ReyEngine::Vec2<R_FLOAT>(minSize.y, maxSize.y).clamp(size.y);
   return {newX, newY};
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Size<R_FLOAT> BaseWidget::getClampedSize(){
   return getClampedSize(getSize());
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> BaseWidget::globalToLocal(const Pos<R_FLOAT>& global, const Pos<R_FLOAT>& localBase) const {
   auto globalPos = getGlobalPos() + localBase;
   auto retval = global - globalPos;
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> BaseWidget::localToGlobal(const Pos<R_FLOAT>& local, const Pos<R_FLOAT>& globalBase) const {
   return local + getGlobalPos() + globalBase;
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<R_FLOAT> BaseWidget::globalToLocal(const Rect<R_FLOAT> &global) const {
   auto globalPos = getGlobalPos();
   auto retval = global - globalPos;
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<R_FLOAT> BaseWidget::localToGlobal(const Rect<R_FLOAT> &local) const {
   return local + getGlobalPos();
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<R_FLOAT> BaseWidget::getChildBoundingBox() const {
   Size<R_FLOAT> childRect;
   for (const auto& child : getChildren()){
      auto totalOffset = child->getRect().size() + Size<R_FLOAT>(child->getPos().x, child->getPos().y);
      childRect = childRect.max(totalOffset);
   }
   return {{0, 0}, {childRect}};
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChain() {
   if (!_visible) return;
   renderBegin();

   rlPushMatrix();
   rlTranslatef(transform.position.x, transform.position.y, 0);
   rlRotatef(transform.rotation * 180/M_PI, 0,0,1);
   rlScalef(transform.scale.x, transform.scale.y, 1);
   for (const auto &child: _backRenderList) {
      child->renderChain();
   }
   render();

   //front render
   for (const auto &child: _frontRenderList) {
      child->renderChain();
   }
   rlPopMatrix();
   renderEnd();
   renderEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseWidget::_process_unhandled_input(const InputEvent& rawEvent, const std::optional<UnhandledMouseInput>& rawMouse) {
   auto passInput = [this](const InputEvent& _event, std::optional<UnhandledMouseInput> _mouse) {
      //iterate backwards since siblings that are towards the end of the orderered child vector are drawn ON TOP of ones prior to them
      for(auto it = getChildren().rbegin(); it != getChildren().rend(); ++it){
         auto& child = *it;
         if (_mouse) {
            //if this is mouse input, make sure it is inside the bounding rect
            switch (_event.eventId) {
               case InputEventMouseMotion::getUniqueEventId():
               case InputEventMouseButton::getUniqueEventId():
               case InputEventMouseWheel::getUniqueEventId():{
                  auto globalPos = _event.toEventType<InputEventMouse>().globalPos;
                  _mouse = child->toMouseInput(globalPos);
                  break;}
            }
         }

         if (child->__process_unhandled_input(_event, _mouse)) {
            return true;
         }
      }
      return false;
   };

   if (!_visible) return false;
   //offset the mouse input if necessary
   auto mouse = rawMouse;
   std::unique_ptr<InputEventMouseUnion> xformedEvent; //this pointer will own the transformed event memory, should it exist.
   if (mouse && hasInputOffset()){
      mouse->localPos += getInputOffset();
      // manually allocate memory for the return value, then transfer it to a unique_ptr for
      // proper lifetime management after transforming it. Avoids unnecessary allocations.
      auto u = new InputEventMouseUnion(rawEvent.toEventType<InputEventMouse>());
      u->mouse.globalPos = u->mouse.globalPos - getInputOffset();
      xformedEvent.reset(u);
   }
   // if the transformed event exists, use it. Otherwise, use the raw event.
   const auto& event = xformedEvent ? xformedEvent->mouse : rawEvent;

   if (_isEditorWidget){
      if (_process_unhandled_editor_input(event, mouse) > 0) return true;
   }
   //apply input masking
   switch(_inputMask.value){
       case NONE:
           break;
       case IGNORE_INSIDE:
           if (mouse && _inputMask.mask.isInside(mouse->localPos)){
               if (_unhandled_masked_input(event.toEventType<InputEventMouse>(), mouse)) return true;
           }
           break;
       case IGNORE_OUTSIDE:
           if (mouse && !_inputMask.mask.isInside(mouse->localPos)) {
               if (_unhandled_masked_input(event.toEventType<InputEventMouse>(), mouse)) return true;
           }
           break;
   }

   auto publishInputEvent = [&]() -> Handled {
      WidgetUnhandledInputEvent unhandledInputEvent(toEventPublisher(), event, mouse);
      publishMutable<WidgetUnhandledInputEvent>(unhandledInputEvent);
      return unhandledInputEvent.handled;
   };

   switch (_inputFilter){
      case InputFilter::INPUT_FILTER_PASS_AND_PROCESS:
         return passInput(event, mouse) || _unhandled_input(event, mouse);
      case InputFilter::INPUT_FILTER_PROCESS_AND_PASS:
         return _unhandled_input(event, mouse) || passInput(event, mouse);
      case InputFilter::INPUT_FILTER_IGNORE_AND_PASS:
         return passInput(event, mouse);
      case InputFilter::INPUT_FILTER_PROCESS_AND_STOP:
         return _unhandled_input(event, mouse);
      case InputFilter::INPUT_FILTER_IGNORE_AND_STOP:
         break;
      case InputFilter::INPUT_FILTER_PUBLISH_AND_PASS:
         if (publishInputEvent()) return true;
         return passInput(event, mouse);
      case InputFilter::INPUT_FILTER_PASS_AND_PUBLISH:
         if (passInput(event, mouse)) return true;
         if (publishInputEvent()) return true;
         break;
      case InputFilter::INPUT_FILTER_PUBLISH_AND_STOP:
         if (publishInputEvent()) return true;
         break;
      case InputFilter::INPUT_FILTER_PASS_PUBLISH_PROCESS:
         if (publishInputEvent()) return true;
         if (_unhandled_input(event, mouse)) return true;
      case InputFilter::INPUT_FILTER_PROCESS_PASS_PUBLISH:
         if (_unhandled_input(event, mouse)) return true;
         if (passInput(event, mouse)) return true;
         if (publishInputEvent()) return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled BaseWidget::_process_unhandled_editor_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   if (!_editor_selected) return false;
   switch(event.eventId){
      case InputEventMouseButton::getUniqueEventId():
      case InputEventMouseMotion::getUniqueEventId():
         auto mouseEvent = event.toEventType<InputEventMouseButton>();
         auto localPos = globalToLocal(mouseEvent.globalPos);
         switch (event.eventId){
            case InputEventMouseButton::getUniqueEventId(): {
               auto mouseButton = event.toEventType<InputEventMouseButton>();
               if (_editor_grab_handles_dragging == -1 && mouseButton.isDown) {
                  //check if we click in the grab handles
                  if (_getGrabHandle(0).isInside(localPos)) {
                     _editor_grab_handles_dragging = 0;
                     return true;
                  }
                  if (_getGrabHandle(1).isInside(localPos)) {
                     _editor_grab_handles_dragging = 1;
                     return true;
                  }
                  if (_getGrabHandle(2).isInside(localPos)) {
                     _editor_grab_handles_dragging = 2;
                     return true;
                  }
                  if (_getGrabHandle(3).isInside(localPos)) {
                     _editor_grab_handles_dragging = 3;
                     return true;
                  }
               } else if (!mouseButton.isDown && _editor_grab_handles_dragging != -1){
                  _editor_grab_handles_dragging = -1;
                  return true;
               }
               break;
            }
            case InputEventMouseMotion::getUniqueEventId():
               auto delta = event.toEventType<InputEventMouseMotion>().mouseDelta;
               //see which handle we are dragging
               auto _newRect = getRect();
               switch(_editor_grab_handles_dragging){
                  case 0:
                     //resize the rect
                     _newRect.x += delta.x;
                     _newRect.y += delta.y;
                     _newRect.width -= delta.x;
                     _newRect.height -= delta.y;
                     setRect(_newRect);
                     return true;
                  case 1:
                     _newRect.y += delta.y;
                     _newRect.height -= delta.y;
                     _newRect.width += delta.x;
                     setRect(_newRect);
                     return true;
                  case 2:
                     _newRect.height += delta.y;
                     _newRect.width += delta.x;
                     setRect(_newRect);
                     return true;
                  case 3:
                     _newRect.x += delta.x;
                     _newRect.width -= delta.x;
                     _newRect.height += delta.y;
                     setRect(_newRect);
                     return true;
                  default:
                     break;
               }
         }
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
UnhandledMouseInput BaseWidget::toMouseInput(const Pos<R_FLOAT> &global) const {
    UnhandledMouseInput childmouse;
    childmouse.localPos = globalToLocal(global);
    childmouse.isInside = isInside(childmouse.localPos);
    return childmouse;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setProcess(bool process) {
   Application::instance().getWindow(0).setProcess(process, toBaseWidget());
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setBackRender(bool _isBackrender) {
   //see if we're in the front render list
   auto ptr = toBaseWidget().get();
   if (_isBackrender) {
      auto found = std::find(_frontRenderList.begin(), _frontRenderList.end(), ptr);
      if (found != _frontRenderList.end()) {
         //remove from front render list
         _frontRenderList.erase(found);
         //add to backrender list
         //TODO: respect sibling order
         _backRenderList.push_back(ptr);
      }
   } else {
      auto found = std::find(_backRenderList.begin(), _backRenderList.end(), ptr);
      if (found != _backRenderList.end()) {
         //remove from front render list
         _backRenderList.erase(found);
         //add to backrender list
         //TODO: respect sibling order
         _frontRenderList.push_back(ptr);
      }
   }
   //nothing to do
   isBackRender = _isBackrender;
}


/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawLine(const ReyEngine::Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawLine(line, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawArrow(const Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA& color, float headSize) const {
    ReyEngine::drawArrow(line, lineThick, color, headSize);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawText(const std::string &text, const Pos<R_FLOAT> &pos) const{
   ReyEngine::drawText(text, pos, getTheme()->font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawText(const std::string &text, const Pos<R_FLOAT> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawText(text, pos, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawText(const std::string& text, const ReyEngine::Pos<R_FLOAT>& pos, const ReyEngine::ReyEngineFont& font, const ReyEngine::ColorRGBA& color, R_FLOAT size, R_FLOAT spacing) const {
   ReyEngine::drawText(text, pos, font, color, size, spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawTextCentered(const std::string &text, const Pos<R_FLOAT> &pos) const{
   ReyEngine::drawTextCentered(text, pos, theme->font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawTextCentered(const std::string &text, const Pos<R_FLOAT> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawTextCentered(text, pos, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawTextCentered(const std::string &text, const ReyEngine::Pos<float> &pos, const ReyEngine::ReyEngineFont &font, const ReyEngine::ColorRGBA &color, float size, float spacing) const {
   ReyEngine::drawTextCentered(text, pos, font, color, size, spacing);
}


/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRectangle(const ReyEngine::Rect<R_FLOAT> &rect, const ReyEngine::ColorRGBA &color) const {
   ReyEngine::drawRectangle(rect, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRectangleLines(const ReyEngine::Rect<R_FLOAT> &rect, float lineThick, const ReyEngine::ColorRGBA &color) const {
   ReyEngine::drawRectangleLines(rect, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRectangleRounded(const ReyEngine::Rect<R_FLOAT> &rect, float roundness, int segments, const ReyEngine::ColorRGBA &color) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<R_FLOAT> newRect(rect + Pos<R_FLOAT>(getGlobalPos()) + Pos<R_FLOAT>(_renderOffset));
   ReyEngine::drawRectangleRounded(rect, roundness, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRectangleRoundedLines(const ReyEngine::Rect<R_FLOAT> &rect, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA &color) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<float> newRect(rect + Pos<R_FLOAT>(getGlobalPos()) + Pos<R_FLOAT>(_renderOffset));
   ReyEngine::drawRectangleRoundedLines(rect, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRectangleGradientV(const ReyEngine::Rect<R_FLOAT> &rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA &color2) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<float> newRect(rect + getGlobalPos() + _renderOffset);
   ReyEngine::drawRectangleGradientV(rect, color1, color2);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawCircle(const ReyEngine::Circle& circle, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawCircle(circle, color);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawCircleLines(const ReyEngine::Circle& circle, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawCircleLines(circle, color);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawCircleSectorLines(const ReyEngine::CircleSector& sector, const ReyEngine::ColorRGBA& color, int segments) const {
   ReyEngine::drawCircleSectorLines(sector, color, segments);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRenderTarget(const ReyEngine::RenderTarget& target, const Pos<R_FLOAT>& dst, const ColorRGBA& tint) const {
   DrawTextureRec(target.getRenderTexture(), {0, 0, (float)target.getSize().x, -(float)target.getSize().y}, {dst.x, dst.y}, tint);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawRenderTargetRect(const ReyEngine::RenderTarget& target, const Rect<R_FLOAT>& src, const Rect<R_FLOAT>& dst, const ColorRGBA& tint) const {
   Rectangle _src = {src.x, -src.y-src.height, (float)src.width, -(float)src.height};
   Rectangle _dst = {dst.x, dst.y, dst.width, dst.height};
   DrawTexturePro(target.getRenderTexture(), _src, _dst, {}, 0, tint);
}

///////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::drawTextureRect(const ReyEngine::ReyTexture& rtex, const ReyEngine::Rect<R_FLOAT> &src, const ReyEngine::Rect<R_FLOAT> &dst, float rotation, const ReyEngine::ColorRGBA &tint) const {
   Rectangle _src  = {src.x, src.y, src.width, src.height};
   Rectangle _dst = {dst.x, dst.y, dst.width, dst.height};
   DrawTexturePro(rtex.getTexture(), _src, _dst, {0,0}, rotation, Colors::none);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::registerProperties() {
   registerProperty(_isProcessed);
   registerProperty(_anchor);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderEditorFeatures(){
   if (_editor_selected){
      //render the grab handles
      static constexpr ReyEngine::ColorRGBA GRAB_HANDLE_COLOR = COLORS::blue;
      drawRectangleRounded(_getGrabHandle(0), 1.0, 5, GRAB_HANDLE_COLOR);
      drawRectangleRounded(_getGrabHandle(1), 1.0, 5, GRAB_HANDLE_COLOR);
      drawRectangleRounded(_getGrabHandle(2), 1.0, 5, GRAB_HANDLE_COLOR);
      drawRectangleRounded(_getGrabHandle(3), 1.0, 5, GRAB_HANDLE_COLOR);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Rect<R_FLOAT> BaseWidget::_getGrabHandle(int index) {
   switch(index){
      case 0:
         return {-GRAB_HANDLE_SIZE,-GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 1:
         return {getWidth(),-GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 2:
         return {getWidth(),getHeight(),GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
      case 3:
         return {-GRAB_HANDLE_SIZE, getHeight(),GRAB_HANDLE_SIZE,GRAB_HANDLE_SIZE};
       default:
           throw std::runtime_error("Invalid grab handle index");
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> BaseWidget::getWidgetAt(Pos<R_FLOAT> pos) {
   //query the widget and figure out which of its children is the topmost widget at the position
   std::shared_ptr<BaseWidget> widgetAt;
   std::function<std::shared_ptr<BaseWidget>(std::shared_ptr<BaseWidget>)> findWidget = [&](std::shared_ptr<BaseWidget> currentWidget){
      if (currentWidget->getGlobalRect().isInside(pos)) widgetAt = currentWidget;
      for (auto& child : currentWidget->getChildren()) {
         if (child->getGlobalRect().isInside(pos)) widgetAt = child;
         findWidget(child);
      }
      return std::shared_ptr<BaseWidget>();
   };
   findWidget(toBaseWidget());
   if (widgetAt) return widgetAt;
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setRect(const ReyEngine::Rect<R_FLOAT>& r){

   auto applyRectProperties = [this](auto& newRect){
      //enforce min/max sizes
      newRect.setSize(newRect.size().max(minSize));
      newRect.setSize(newRect.size().min(maxSize));
      const auto& oldRect = getRect();
      applyRect(newRect);
      __on_rect_changed(oldRect);
      _publishSize();
   };

   ReyEngine::Rect<R_FLOAT> newRect(r);
   auto parent = getParent().lock();
   int parentHeight, parentWidth;
   if(parent && getAnchoring() != Anchor::NONE) {
      parentHeight = parent->getHeight();
      parentWidth = parent->getWidth();
      newRect = {{0, 0}, {parentWidth, parentHeight}};
   } else if (isRoot()) {
      auto windowSize = Application::instance().windowCount() ? Application::instance().getWindow(0).getSize() : Size<int>(0,0);
      newRect = {{0, 0}, windowSize};
   } else {
      applyRectProperties(newRect);
      return;
      //todo: this is weird, find a way to make this flow better
   }
   // enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   switch (_anchor.value) {
      case Anchor::NONE:
          newRect = r;
         break;
      case Anchor::FILL: {
         //take up as much space as parent has to offer
         break;
      }
      case Anchor::LEFT: {
          //Place on the center left of the parent
          newRect = {{0, parentHeight / 2 - getHeight() / 2}, getSize()};
          break;
      }
      case Anchor::RIGHT: {
          //Place on the center right of the parent
          newRect = {{parentWidth - getWidth(), parentHeight / 2 - getHeight() / 2}, getSize()};
          break;
      }
      case Anchor::TOP: {
          //Place at the center top of the parent
          newRect = {{parentWidth / 2 - getWidth() / 2, 0}, getSize()};
          break;
      }
      case Anchor::BOTTOM: {
          //Place at the center bottom of the parent
          newRect = {{parentWidth / 2 - getWidth() / 2, parentHeight - getHeight()}, getSize()};
          break;
      }
      case Anchor::TOP_LEFT: {
          //Place at the top left of the parent
          newRect = {{0, 0}, getSize()};
          break;
      }
      case Anchor::TOP_RIGHT: {
          //Place at the top right of the parent
          newRect = {{parentWidth - getWidth(), 0}, getSize()};
          break;
      }
      case Anchor::BOTTOM_RIGHT: {
          //Place at the bottom right of the parent
          newRect = {{parentWidth - getWidth(), parentHeight - getHeight()}, getSize()};
          break;
      }
      case Anchor::BOTTOM_LEFT: {
          //Place at the bottom left of the parent
          newRect = {{0, parentHeight - getHeight()}, getSize()};
          break;
      }
      case Anchor::CENTER: {
          //Place at the center of the parent
          newRect = {{parentWidth / 2 - getWidth() / 2, parentHeight / 2 - getHeight() / 2}, getSize()};
          break;
      }

      default:
         break;
   }
   applyRectProperties(newRect);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setMinSize(const ReyEngine::Size<R_FLOAT>& size) {
    minSize = size;
    setRect(getRect());
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setMaxSize(const ReyEngine::Size<R_FLOAT>& size) {
    maxSize = size;
    setRect(getRect());
}
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setPos(R_FLOAT x, R_FLOAT y){
   ReyEngine::Rect<R_FLOAT> r(x, y, getRect().width, getRect().height);
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setPos(const Pos<R_FLOAT>& pos) {
   ReyEngine::Rect<R_FLOAT> r(pos, getRect().size());
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setX(R_FLOAT x) {
   Rect<R_FLOAT> r({x, getRect().pos().y}, getRect().size());
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setY(R_FLOAT y){
   Rect<R_FLOAT> r({getRect().pos().x, y}, getRect().size());
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setPosRelative(const Pos<R_FLOAT>& pos, const Pos<R_FLOAT>& basis) {
   setPos(pos - basis);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::move(const Pos<R_FLOAT> &amt) {
   setPos(getRect().pos() + amt);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setSize(const ReyEngine::Size<R_FLOAT>& size){
   ReyEngine::Rect<R_FLOAT> r(getRect().pos(), size);
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setAnchoring(Anchor newAnchor) {
   if (isInLayout){
      Logger::error() << getScenePath() << ": Children of layouts cannot have anchoring!";
      return;
   }
   _anchor.value = newAnchor;
   auto parent = getParent().lock();
   if (parent){
      setRect(getRect());
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::scale(const Vec2<R_FLOAT> &scale) {
   Size<R_FLOAT> newSize = {getWidth() * scale.x, getHeight() * scale.y};
   setSize(newSize);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setWidth(R_FLOAT width){
   ReyEngine::Rect<R_FLOAT> r(getRect().pos(), {width, getRect().height});
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setHeight(R_FLOAT height){
   ReyEngine::Rect<R_FLOAT> r(getRect().pos(), {getRect().width, height});
   setRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::__on_rect_changed(const Rect<R_FLOAT>& oldRect){
   if (oldRect.size() != getSize()) {
      for (auto &child: getChildren()) {
         if (child->isAnchored()) {
            child->setRect(child->getRect());
         }
      }
   }
   //notify parents
   auto parent = getParent().lock();
   if (parent){
      auto widget = toBaseWidget();
      parent->_on_child_rect_changed(widget);
   }
   _on_rect_changed();
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setFocus(bool isFocus) {
   auto canvas = getCanvas();
   if (!canvas){
      std::runtime_error("Error: BaseWidget " + getName() + " does not belong to a canvas!");
   }
   if (isFocus) {
      auto thiz = toBaseWidget();
      canvas.value()->setFocus(thiz);
   } else {
      canvas.value()->clearFocus();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setModal(bool isModal) {
   auto canvas = getCanvas();
   if (!canvas){
      std::runtime_error("Error: BaseWidget " + getName() + " does not belong to a canvas!");
   }
   if (isModal) {
      auto thiz = toBaseWidget();
      canvas.value()->setModal(thiz);
   } else {
      canvas.value()->clearModal();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<ReyEngine::Canvas>> BaseWidget::getCanvas() {
   auto weakPrt = getParent();
   while (!weakPrt.expired()) {
      auto parent = weakPrt.lock();
      if (!parent) {
         return nullopt;
      }
      if (parent->_get_static_constexpr_typename() == ReyEngine::Canvas::TYPE_NAME) {
         return parent->toType<ReyEngine::Canvas>();
      }
      weakPrt = parent->getParent();
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::startScissor(const ReyEngine::Rect<R_FLOAT>& area) const {
   auto& thiz = const_cast<BaseWidget&>(*this);
   auto canvasOpt = thiz.getCanvas();
   if (canvasOpt){
      auto scissorRect = area;
      scissorRect.setPos(localToGlobal(area.pos()));
      canvasOpt.value()->pushScissor(scissorRect);
   }

}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::stopScissor() const {
   auto& thiz = const_cast<BaseWidget&>(*this);
   auto canvasOpt = thiz.getCanvas();
   if (canvasOpt){
      canvasOpt.value()->popScissor();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> BaseWidget::askHover(const Pos<R_FLOAT>& globalPos) {
    //ask this widget to accept the hover

    auto process = [&]() -> std::optional<std::shared_ptr<BaseWidget>> {
        if (acceptsHover && isInside(globalToLocal(globalPos) - getInputOffset())){
            return toBaseWidget();
        }
        return nullopt;
    };

    auto pass = [&]() -> std::optional<std::shared_ptr<BaseWidget>>{
            for (auto it = getChildren().rbegin(); it != getChildren().rend(); ++it) {
                const auto &child = *it;
                auto globalPosCopy = globalPos;
                if (hasInputOffset()){
                   globalPosCopy -= getInputOffset();
                }
               auto handled = child->askHover(globalPosCopy);
               if (handled) return handled;
            }
        return nullopt;
    };
//            Logger::debug() << "Asking widget " << widget->getName() << " to accept hover " << endl;
    if (!_visible) return nullopt;
    std::optional<std::shared_ptr<BaseWidget>> handled;
    switch (_inputFilter) {
        case InputFilter::INPUT_FILTER_PROCESS_AND_STOP:
            return process();
        case InputFilter::INPUT_FILTER_PROCESS_AND_PASS:
            handled = process();
            if (handled) return handled;
            return pass();
        case InputFilter::INPUT_FILTER_PASS_AND_PROCESS:
            handled = pass();
            if (handled) return handled;
            return process();
        case InputFilter::INPUT_FILTER_IGNORE_AND_STOP:
            return nullopt;
        case InputFilter::INPUT_FILTER_IGNORE_AND_PASS:
            return pass();
        default:
            return nullopt;
    }
};
///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::__on_added_to_parent() {}

///////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::__on_enter_tree() {
   auto me = inheritable_enable_shared_from_this<Component>::downcasted_shared_from_this<BaseWidget>().get();
   auto parent = getParent().lock();
   isInLayout = parent->isLayout;
   //recalculate the size rect if need to
   //todo: fix size published twice (setrect and later _publishSize
   if (isAnchored() || isLayout){
      //anchoring and layout of children managed by this component
      setRect(getRect());
   }
   if (isInLayout){
      //placement of layout managed by parent
      parent->setRect(parent->getRect());
   }
   if (_has_inited) {
      _publishSize();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::__on_child_removed(Internal::TypeContainer<BaseWidget>::ChildPtr& child) {
   //remove from renderlist
   auto frontRenderFound = std::find(_frontRenderList.begin(), _frontRenderList.end(), child->toBaseWidget().get());
   if (frontRenderFound != _frontRenderList.end()) {
      _frontRenderList.erase(frontRenderFound);
   } else {
      auto backRenderFound = std::find(_backRenderList.begin(), _backRenderList.end(), child->toBaseWidget().get());
      if (backRenderFound != _backRenderList.end()){
         _backRenderList.erase(backRenderFound);
      }
   }
   child->toContainedType().Positionable2D<R_FLOAT>::setParent(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::__on_child_added_immediate_basewidget(ChildPtr& child) {
   if (child->isBackRender.value){
      _backRenderList.push_back(child.get());
      if (BaseWidget::verbose)
         Logger::debug() << "Child " << child->getName() << " added to parent " << getName() << "'s backrender list" << endl;
   } else {
      _frontRenderList.push_back(child.get());
      if (BaseWidget::verbose)
         Logger::debug() << "Child " << child->getName() << " added to parent " << getName() << "'s frontrender list" << endl;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
template <>
void ReyEngine::Internal::TypeContainer<BaseWidget>::__on_child_added_immediate(std::shared_ptr<BaseWidget>& child){
   if(_get_static_constexpr_typename() != Window::TYPE_NAME){
      //window will be typecontainer root, so we don't want to make it the parent since its not truly a positionable
      auto mypositionable = static_cast<Positionable2D<R_FLOAT>*>(&toContainedType());
      child->toContainedType().Positionable2D<R_FLOAT>::setParent(mypositionable);
   }
   child->__init();
   if (_get_static_constexpr_typename() != Window::TYPE_NAME) {
      toContainedType().__on_child_added_immediate_basewidget(child);
   }
   _on_child_added(child);
}