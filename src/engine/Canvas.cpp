#include "Canvas.h"

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
   if (!_visible) return;
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
Handled Canvas::_processInput(const InputEvent& event) {
   if (!_visible) return nullptr;
   // if (_inputContext) {
   //    auto handled = _inputContext->handleInput(event);
   //    if (handled) return handled;
   // }

   auto isMouse = event.isMouse(); //cache this for speed

   //lets us catch and do stuff with unhandled input on the canvas level
   // only reacts to handled input, after it has fallen out of scope
   struct HandledInputInterceptor {
      HandledInputInterceptor(Canvas* canvas, const InputEvent& event)
      : canvas(canvas)
      , event(event)
      , cachedMouseData(event.isMouse() ? *event.isMouse().value() : decltype(cachedMouseData)())
      {}
      ~HandledInputInterceptor(){
         if (!handled) return;
         switch (event.eventId){
            case InputEventMouseToolTip::ID:{
               auto& ttEvent = event.toEvent<InputEventMouseToolTip>();
               if (ttEvent.isCancel) {
                  canvas->setToolTip(nullptr);
               } else if (!handled.handler->getToolTipText().empty()) {
                  // Set tooltip on the handler's own canvas, fallback to interceptor's canvas
                  if (auto handlerCanvas = handled.handler->getCanvas()) {
                     handlerCanvas.value()->setToolTip(handled.handler);
                  } else {
                     canvas->setToolTip(handled.handler);
                  }
               }
               break;}
            case InputEventMouseMotion::ID:{
               canvas->setToolTip(nullptr);
               break;}
            case InputEventMouseHover::ID:{
               // Set hover on the handler's own canvas, fallback to interceptor's canvas
               if (auto handlerCanvas = handled.handler->getCanvas()) {
                  handlerCanvas.value()->setHover(handled.handler);
               } else {
                  canvas->setHover(handled.handler);
               }
               break;}
         }
         //return mouse data to its original state
         if (auto mouseData = event.isMouse()) {
            *mouseData.value() = cachedMouseData.value();
         }
      }
      HandledInputInterceptor& operator=(const Handled& h){handled = h; return *this;}
      operator bool(){return handled;}
      operator Handled&(){return handled;}

   private:
      Handled handled = nullptr;
      Canvas* canvas = nullptr;
      std::optional<const MouseEvent> cachedMouseData;
      const InputEvent& event;
   };

   HandledInputInterceptor handled(this, event);

   //query modal widgets first. A modal widget consumes input even if unhandled and prevents anyone else from getting it.
   if (auto modal = getModal()){
      std::unique_ptr<MouseEvent::ScopeTransformer> scopeXformer;
      if (event.isMouse()) {
         //subtract off the modal widget's transform to avoid double-apply
         auto modalXform = modal->getCanvasTransform().get();
         scopeXformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), modalXform, modal->size, getCameraTransform());
      }
      handled = modal->_processInput(event);
      if (modal->_handleAllModalInput || handled) return handled;
   }

   //then focused widgets
   if (auto focused = getFocus()){
      std::unique_ptr<MouseEvent::ScopeTransformer> scopeXformer;
      if (event.isMouse()) {
         //subtract off the focus widget's transform to avoid double-apply
         auto focusTransform = focused->getCanvasTransform().get();
         // Only apply camera transform for background widgets, not foreground
         bool isForeground = _foreground.contains(focused->getNode());
         if (isForeground) {
            scopeXformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), focusTransform, focused->size);
         } else {
            scopeXformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), focusTransform, focused->size, getCameraTransform());
         }
      }
      handled = focused->_processInput(event);
      if (handled) return handled;
   }

   //outside input should still propogate to focused, modal, and foreground widgets
   if (_ignoreOutsideInput && !isFocused() && !isModal()) {
      if (isMouse && !isMouse.value()->isInside()){
         return nullptr;
      }
   }

   //then foreground (which is unaffected by camera transform)
   for (auto& child : _foreground.getValues() | std::views::reverse) {
      if (auto widget = child->as<Widget>()){
         //transform mouse input into our own coordinate space
         std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
         if (event.isMouse()) {
            // Logger::debug() << w->getName() << " local pos before = " << e.isMouse().value()->getLocalPos() << (e.isMouse().value()->isInside() ? " inside " : "")  << endl;
            xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), widget.value()->getLocalTransform(), widget.value()->getSize());
         }
         handled = widget.value()->_processInput(event);
         if (handled) return handled;
      }
   }

   //then background (which is affected by camera transorm)
   // this here is "normal" input
   for (auto& child : _background.getValues() | std::views::reverse) {
      if (auto isWidget= child->as<Widget>()) {
         std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
         if (isMouse){
            xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), isWidget.value()->getLocalTransform(), isWidget.value()->size, getCameraTransform());
         }
         handled = isWidget.value()->_processInput(event);
         if (handled) return handled;
      }
   }

   //penultimately, we attempt to handle it ourselves as foreground input
   handled = __process_unhandled_input(event);
   if (handled) return handled;

   //finally we publish the input
   WidgetUnhandledInputEvent unhandledInputEvent(this, event);
   publishMutable(unhandledInputEvent);
   return unhandledInputEvent.handled;
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
Canvas::BackgroundSpace Canvas::toBackgroundPos(const Pos<float>& p) const {
   return (Pos<float>)GetScreenToWorld2D(Vector2(p), getCamera());
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<float> Canvas::toForegroundPos(const BackgroundSpace& p) const {
   return (Pos<float>)GetWorldToScreen2D(Vector2(p.get()), getCamera());
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Pos<float>> Canvas::toChildPos(const Widget* child, const Pos<float>& p) const {
   if (isInBackground(child)) {
      return toBackgroundPos(p).get().transform(child->getLocalTransform().matrix);
   }
   if (isInForeground(child)) {
      return p.transform(child->getLocalTransform().matrix);
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Transform2D> Canvas::getChildXform(const Widget* child) const {
   if (isInBackground(child)) {
      return getCameraTransform() * child->getLocalTransform();
   }
   if (isInForeground(child)) {
      return child->getLocalTransform();
   }
   return {};
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