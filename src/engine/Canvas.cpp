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
   if (widget->as<Canvas>()) return;
   context.push(widget->transform2D);
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

   //draw the modal widget and place it where it needs to go
   if (auto modal = getModal()){
      if (auto modalDrawable = modal->_node->as<Widget>()) {
         //find the canvas transform of the modal widget
         doRenderModal(renderContext, modalDrawable.value());
      }
   }

   // rlPopMatrix(); // TODO: investigate what is pushing this - seems to be needed but no matching push is visible

   //root canvas has no parent canvas. So ensure root canvas draws its foreground.
   if (!getCanvas()) {
      for (auto& child : _foreground.getValues()){
         if (auto widget = child->as<Widget>()) {
            doRender(renderContext, widget.value());
         }
      }
   }

   // //finally, draw tooltip
   // if (auto toolTip = getToolTip()){
   //    rlPushMatrix();
   //    auto windowPos = InputManager::getMousePos();
   //    auto localPos = windowToLocalPos(windowPos);
   //    auto toolTipText = toolTip->getToolTipText();
   //    auto textSize = measureText(toolTipText, theme->font);
   //    static constexpr float embiggenness = 4;
   //    Rect<float> toolTipRect = Rect<float>(localPos + Pos<float>(20,20), textSize).embiggen(embiggenness);
   //    drawRectangle(toolTipRect, Colors::white);
   //    drawText(toolTipText, toolTipRect.topLeft() + Pos<float>(embiggenness, embiggenness), theme->font);
   //    drawRectangleLines(toolTipRect, 1.0, Colors::black);
   //    rlPopMatrix();
   // }
}

/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<float>> Canvas::getMousePos() {
   return InputManager::getMousePos().get();
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
      handled = processNode<InputProcess>(modal->_node, true, event);
      if (modal->_handleAllModalInput || handled) return handled;
   }

   //then focused widgets
   if (auto focus = getFocus()){
      if (event.isMouse() && focus->canvas != this) {
         //subtract off our own global transform in case this focus was propogated from a subcanvas
         auto xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), focus->canvas->getGlobalTransform(false).get(), size);
         handled = processNode<InputProcess>(focus->canvas->_node, true, event);
      } else {
         //this version makes sure we don't get stuck in an infinite loop since the focused node's canvas could be us
         handled = processNode<InputProcess>(focus->_node, true, event);
      }
      if (handled) return handled;
   }

   //then foreground (which is unaffected by camera transform)
   for (auto& child : _foreground.getValues() | std::views::reverse) {
      if (auto widget = child->as<Widget>()){
         handled = processNode<InputProcess>(child, false, event);
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
      std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
      auto widget = child->as<Widget>();
      if (isMouse){
         if (!widget) continue;
         xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), getLocalTransform(), widget.value()->size, getCameraTransform());
      }
      handled = processNode<InputProcess>(child, false, event);
      if (handled) return handled;
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