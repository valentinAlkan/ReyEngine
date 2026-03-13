#include "Canvas.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_init() {

}

////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__on_child_added_to_tree(TypeNode *child) {
   //move all nodes to background by default
   _background.add(child);
   ReyObject::__on_child_added_to_tree(child);
}

////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__on_child_removed_from_tree(TypeNode *n) {
   cout << "REMOVING FORM TREE!" << endl;
   _foreground.remove(n);
   _background.remove(n);
   ReyObject::__on_child_removed_from_tree(n);
}


////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__on_rect_changed(const Rect<float> &oldRect, const Rect<float> &newRect, bool allowsAnchor, bool byLayout) {
   Widget::__on_rect_changed(oldRect, newRect, allowsAnchor, byLayout);
   if (oldRect.size() != newRect.size()) {
      _renderTarget.setSize(getSize());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__on_descendant_added_to_tree(TypeNode *n) {
   static constexpr char INDENT = '-';
   string indent;
   function<void(TypeNode*)> catTree = [&](TypeNode* n){
      cout << indent << n->name << "" << endl;
      indent += INDENT;
      for (const auto& child : n->getChildren()){
         catTree(child);
      }
      indent.resize(indent.length() - sizeof(INDENT)/sizeof(char));
   };
//   cout << "------------------------" << endl;
//   catTree(_node);
   //assign the object to its owning canvas (if not done so already)
   if (auto isDrawable = n->as<Drawable2D>()){
      if (!isDrawable.value()->canvas) {
         isDrawable.value()->canvas = this;
      }
   }
   //make sure to call dispatchers
   ReyObject::__on_descendant_added_to_tree(n);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__on_descendant_removed_from_tree(TypeNode *n) {
   //remove all statuses from widgets
   if (auto widget = n->as<Widget>()){
      _removeAllStatus(widget.value());
   }

   ReyObject::__on_descendant_removed_from_tree(n);
}

////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_removeAllStatus(Widget* widget) {
   auto removeStatusForType = [this, widget]<typename StatusType>() {
      constexpr std::size_t statusIndex = WidgetStatus::tuple_type_index_v<StatusType, WidgetStatus::StatusTypes>;
      if (statusWidgetStorage[statusIndex] == widget) {
         setStatus<StatusType>(nullptr);
      }
   };

   //use std apply to unpack each status type, instantiate it, and pass it to the lambda
   std::apply([&removeStatusForType](auto... statusTypes) {(removeStatusForType.template operator()<decltype(statusTypes)>(), ...);}, WidgetStatus::StatusTypes{});
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::doRender(RenderContext& context, Widget* widget, bool isModalRenderChain) {
   if (!widget->_visible) return;
   context.push(widget->transform2D);
   if (auto canvas = widget->as<Canvas>()) {
      //freeze the render context and switch to the new texture
      auto subCanvas = canvas.value();
      {
         auto frozenContext = context.freeze();
         {
            //scope desctruction control
            RenderContext newContext(subCanvas->_renderTarget);
            canvas.value()->renderProcess(newContext);
         }
      } //automatically unfreeze and revert the render context
      //draw the render target
      drawRenderTargetRect(subCanvas->readRenderTarget(), subCanvas->getSizeRect(), subCanvas->getSizeRect(), Colors::none);
   } else {
      if (widget->_modal && !isModalRenderChain) {
         context.recordModal();
      } else {
         widget->render2DBegin();
         widget->render2D();
         widget->render2DEnd();
      }
      for (auto& child : widget->getChildrenAs<Widget>()) {
         doRender(context, child);
      }
   }
   context.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::doRenderModal(RenderContext& context, Widget* widget) {
   context.push(context.getModalTransform());
   widget->render2DBegin();
   widget->render2D();
   widget->render2DEnd();
   for (auto& child : widget->getChildrenAs<Widget>()) {
      doRender(context, child, true);
   }
   context.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::renderProcess(RenderContext& renderContext) {
   if (!_retained) {
      ClearBackground(Colors::lightGray);
   }

   render2DBegin();
   render2D();
   render2DEnd();

   //render background elements, which are affected by camera transform
   {
      auto cameraMode = renderContext.cameraContext(camera);
      for (auto& child : _background.getValues()){
         if (auto widget = child->as<Widget>()) {
            doRender(renderContext, widget.value());
         }
      }
   }

   //draw foreground elements
   for (auto& child : _foreground.getValues()){
      if (auto widget = child->as<Widget>()) {
         doRender(renderContext, widget.value());
      }
   }

   //draw the modal widget and place it where it needs to go
   if (auto modal = getModal()){
      if (auto modalDrawable = modal->_node->as<Widget>()) {
         //find the canvas transform of the modal widget
         doRenderModal(renderContext, modalDrawable.value());
      }
   }

   //finally, draw tooltip
   if (auto toolTip = getToolTip()){
      auto windowPos = InputManager::getMousePos();
      auto localPos = windowToLocalPos(windowPos);
      auto toolTipText = toolTip->getToolTipText();
      auto textSize = measureText(toolTipText, theme->font);
      static constexpr float embiggenness = 4;
      Rect<float> toolTipRect = Rect<float>(localPos + Pos<float>(20,20), textSize).embiggen(embiggenness);
      drawRectangle(toolTipRect, Colors::white);
      drawText(toolTipText, toolTipRect.topLeft() + Pos<float>(embiggenness, embiggenness), theme->font);
      drawRectangleLines(toolTipRect, 1.0, Colors::black);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<float>> Canvas::getMousePos() {
   return InputManager::getMousePos().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::pass(Widget* currentWidget, const InputEvent& event, bool isModal) {
   for (auto& child : currentWidget->getChildrenAs<Widget>()) {
      if (child->isModal()) {
         //we will come back to this later
         continue;
      } else {
         auto handled = doInput(child, event, isModal);
         if (handled) return handled;
      }
   }
   return nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::publish(Widget* currentWidget, const InputEvent& event, bool isModal) {
   WidgetUnhandledInputEvent _event(currentWidget, event);
   currentWidget->publishMutable(_event);
   return _event.handler;
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::process(Widget* currentWidget, const InputEvent& event, bool isModal){
   return currentWidget->__process_unhandled_input(event);
}
/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::doInput(Widget* w, const InputEvent& e, bool isModal){
   //transform mouse input into the child's space
   std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
   if (e.isMouse()) {
      // Logger::debug() << w->getName() << " local pos before = " << e.isMouse().value()->getLocalPos() << endl;
      xformer = make_unique<MouseEvent::ScopeTransformer>(*e.isMouse().value(), w->getLocalTransform(), w->getSize());
      // Logger::debug() << w->getName() << " local pos after = " << e.isMouse().value()->getLocalPos() << (e.isMouse().value()->isInside() ? " inside " : "")  << endl;
   }
   if (!w->isModal() || isModal) {
      //ignore outside input if applicable
      auto shouldIgnore = [](const Widget& widget, const Pos<float>& pos){
         return widget._ignoreOutsideInput && !widget.getSizeRect().contains(pos);
      };
      if (auto mouse = e.isMouse()) {
         auto localPos = mouse.value()->getLocalPos();
         if (shouldIgnore(*w, localPos)) return nullptr;
      }
   }
   #define RETURN if (handled) return handled
   Widget* handled = nullptr;
   switch(w->_inputFilter) {
      case InputFilter::PASS_ONLY: handled = pass(w, e, isModal); RETURN; break;
      case InputFilter::PROCESS_ONLY: handled = process(w, e, isModal); RETURN; break;
      case InputFilter::PUBLISH_ONLY: handled = publish(w, e, isModal); RETURN; break;
      case InputFilter::PASS_AND_PROCESS: handled = pass(w, e, isModal); RETURN; handled = process(w, e, isModal); RETURN; break;
      case InputFilter::PROCESS_AND_PASS: handled = process(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; break;
      case InputFilter::PROCESS_AND_STOP: handled = process(w, e, isModal); RETURN; return this;
      case InputFilter::PROCESS_AND_PUBLISH: handled = process(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; break;
      case InputFilter::IGNORE_AND_PASS: handled = pass(w, e, isModal); RETURN; break;
      case InputFilter::IGNORE_AND_STOP: return this;
      case InputFilter::PUBLISH_AND_PASS: handled = publish(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; break;
      case InputFilter::PASS_AND_PUBLISH: handled = pass(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; break;
      case InputFilter::PUBLISH_AND_STOP: handled = publish(w, e, isModal); RETURN; return this;
      case InputFilter::PASS_PUBLISH_PROCESS: handled = pass(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; handled = process(w, e, isModal); RETURN; break;
      case InputFilter::PASS_PROCESS_PUBLISH: handled = pass(w, e, isModal);  RETURN; handled = process(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; break;
      case InputFilter::PASS_PROCESS_STOP: handled = pass(w, e, isModal);  RETURN; handled = process(w, e, isModal); RETURN; return this;
      case InputFilter::PROCESS_PUBLISH_PASS: handled = process(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; break;
      case InputFilter::PROCESS_PASS_PUBLISH: handled = process(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; handled = publish(w, e, isModal); RETURN; break;
      case InputFilter::PUBLISH_PASS_PROCESS: handled = publish(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; handled = process(w, e, isModal); RETURN; break;
case InputFilter::PUBLISH_PROCESS_PASS: handled = publish(w, e, isModal); RETURN; handled = process(w, e, isModal); RETURN; handled = pass(w, e, isModal); RETURN; break;
   }
   #undef RETURN
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::__process_unhandled_input(const InputEvent& event) {
   // if (_inputContext) {
   //    auto handled = _inputContext->handleInput(event);
   //    if (handled) return handled;
   // }

   auto isMouse = event.isMouse(); //cache this for speed

   //lets us catch and do stuff with handlers before they fall out of scope. Basically lets us query arbitrary widgets
   // to see if they want to respond to an event
   struct EventHandler {
      EventHandler(Canvas* canvas, const InputEvent& event)
      : canvas(canvas)
      , event(event)
      , cachedMouseData(event.isMouse() ? *event.isMouse().value() : decltype(cachedMouseData)())
      {}
      ~EventHandler(){
         if (!handler) return;
         switch (event.eventId){
            case InputEventMouseToolTip::ID:{
               auto& ttEvent = event.toEvent<InputEventMouseToolTip>();
               if (ttEvent.isCancel) {
                  canvas->setToolTip(nullptr);
               } else if (!handler->getToolTipText().empty()) {
                  canvas->setToolTip(handler);
               }
               break;}
            case InputEventMouseMotion::ID:{
               canvas->setToolTip(nullptr);
               break;}
            case InputEventMouseHover::ID:{
               canvas->setHover(handler);
               break;}
         }
         //return mouse data to its original state
         if (auto mouseData = event.isMouse()) {
            *mouseData.value() = cachedMouseData.value();
         }
      }
      EventHandler& operator=(Widget* w){handler = w; return *this;}
      operator bool(){return handler!=nullptr;}
      operator Widget*(){return handler;}
   private:
      Widget* handler = nullptr;
      Canvas* canvas = nullptr;
      std::optional<const MouseEvent> cachedMouseData;
      const InputEvent& event;
   };

   EventHandler handled(this, event);

   //query modal widgets first. A modal widget consumes input even if unhandled and prevents anyone else from getting it.
   if (auto modal = getModal()){
      handled = doInput(modal, event, true);
      if (modal->_handleAllModalInput || handled) return handled;
   }

   //then focused widgets
   if (auto focused = getFocus()){
      std::unique_ptr<MouseEvent::ScopeTransformer> scopeXformer;
      if (event.isMouse()) {
         //figure out the transform for the focused element
         auto parent = focused->getParentWidget();
         Transform2D transform = focused->transform2D;
         while (parent && parent.value() != this) {
            transform *= parent.value()->getTransform();
            parent = parent.value()->getParentWidget();
         }
         scopeXformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), transform, focused->size, getCameraTransform());
      }
      handled = doInput(focused, event, true);
      if (handled) return handled;
   }

   //then foreground (which is unaffected by camera transform)
   for (auto& child : _foreground.getValues() | std::views::reverse) {
      if (auto widget = child->as<Widget>()){
         handled = doInput(widget.value(), event, false);
         if (handled) return handled;
      }
   }

   //outside input should still propogate to focused, modal, and foreground widgets
   if (_ignoreOutsideInput) {
      if (isMouse && !getSizeRect().contains(isMouse.value()->getLocalPos() - getPos())){
         return nullptr;
      }
   }

   //then background (which is affected by camera transorm)
   // this here is "normal" input
   for (auto& child : _background.getValues() | std::views::reverse) {
      if (auto isWidget= child->as<Widget>()) {
         std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
         if (isMouse){
            xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), MatrixIdentity(), isWidget.value()->size, getCameraTransform());
         }
         handled = doInput(isWidget.value(), event, false);
         if (handled) return handled;
      }
   }

   //finally we attempt to handle it ourselves as foreground input, but we need to handjam some values since that never
   // gets set. There's probably a specific transform one could pass to scope transformer, but damned if i know what it is.
   // also, copy the event in case it needs to be used elsewhere, we won't have screwed it up
   auto newEvent = event;
   if (auto newEventMouse = newEvent.isMouse()){
      auto mousePos = isMouse.value()->getLocalPos() - getPos();
//      newEventMouse.value()->setCanvasPos(mousePos);
      newEventMouse.value()->setLocalPos(mousePos);
      newEventMouse.value()->setIsInside(getSizeRect().contains(mousePos));
   }
   return Widget::__process_unhandled_input(event);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
optional<TypeNode*> moveLayerSafetyChecks(Canvas* canvas, Widget* widget, const std::string& fxName){
   //check canvas is in good state
   if (!canvas){
      Logger::error() << "Canvas: " << fxName << " - Null canvas!" << endl;
      return {};
   }

   //check null widget
   if (!widget) {
      Logger::error() << "Canvas: " << fxName << " - Null widget!" << endl;
      return {};
   }

   //check null node
   auto node = widget->getNode();
   if (!node) {
      Logger::error() << "Canvas: " << fxName << " - Null node!" << endl;
      return {};
   }

   //ensure the widget is a child of the canvas
   TypeNode* parent = node->getParent();
   if (!parent){
      Logger::error() << "Canvas (" << canvas->getName() << "): " << fxName << " - widget " << node->getName() << " must be a child of this canvas at this time" << endl;
      return {};
   } else if (node->getParent() != canvas->getNode()){
      Logger::error() << "Canvas (" << canvas->getName() <<  "): " << fxName << " - widget " << node->getName() << " must be a child of this canvas" << endl;
      return {};
   }
   return node;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::moveToForeground(Widget* widget) {
   if (auto node = moveLayerSafetyChecks(this, widget, "moveToForeground")) {
      _background.remove(node.value());
      _foreground.add(node.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::moveToBackground(Widget* widget) {
   if (auto node = moveLayerSafetyChecks(this, widget, "moveToBackground")) {
      _foreground.remove(node.value());
      _background.add(node.value());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::TransformStack::pushTransform(Transform2D* transform2D) {
   rlMultMatrixf(MatrixToFloat(transform2D->matrix));
   globalTransformStack.push(transform2D);
   globalTransform = rlGetMatrixTransform();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::TransformStack::popTransform() {
   rlMultMatrixf(MatrixToFloat(globalTransformStack.top()->inverse().matrix));
   globalTransformStack.pop();
   globalTransform = rlGetMatrixTransform();
}