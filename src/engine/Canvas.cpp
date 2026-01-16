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
      //assign the object to its owning window
      isDrawable.value()->window = window;
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
void Canvas::renderProcess(RenderTarget& parentTarget) {
   if (!_visible) return;
   if (&parentTarget != &_renderTarget) {
      parentTarget.endRenderMode();
   }
   _renderTarget.beginRenderMode();
   rlPushMatrix();
   render2DBegin();

   if (!_retained) {
      ClearBackground(Colors::lightGray);
   }
   render2DBegin();
   render2D();
   render2DEnd();

   BeginMode2D(camera);
   for (auto& child : _background.getValues()){
      processNode<RenderProcess>(child, false);
   }
   EndMode2D();

   //the modal widget's xform includes canvas xform, so we want to pop that off as if we are rendering globally
   if (auto modal = getModal()){
      auto modalDrawable = modal->_node->as<Widget>();
      transformStack.pushTransform(&getModal()->getGlobalTransform().get());

      //invert (subtract off) the modal widget's own position since it's already encoded in modalXform.
      auto inverseXform = modalDrawable.value()->getTransform().inverse();
      transformStack.pushTransform(&inverseXform);

      processNode<RenderProcess>(modal->_node, true);
      transformStack.popTransform();
      //there might need to be an extra pop here
   }

   rlPopMatrix();

   //root canvas has no parent canvas. So ensure root canvas draws its foreground.
   if (!getCanvas()) {
      rlPushMatrix();
      for (auto& child : _foreground.getValues()){
         processNode<RenderProcess>(child, false);
      }
      rlPopMatrix();
   }

   //finally, draw tooltip
   if (auto toolTip = getToolTip()){
      rlPushMatrix();
//      drawRectangle({toolTip->getPos(), Size<float>(100,20)}, Colors::black);
      rlPopMatrix();
   }

   _renderTarget.endRenderMode();
   //return render control to the parent canvas, if any
   if (&parentTarget != &_renderTarget) {
      parentTarget.beginRenderMode();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<float>> Canvas::getMousePos() {
   return InputManager::getMousePos().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::__process_hover(const InputEventMouseHover& event){
   auto& _event = const_cast<InputEventMouseHover&>(event); //should not be rvalue
   auto accepted = __process_unhandled_input(_event);
   setHover(accepted);
   return accepted;
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::__process_unhandled_input(const InputEvent& event) {
   auto isMouse = event.isMouse(); //cache this for speed

   //lets us catch and do stuff with handlers before they fall out of scope. Basically lets us query arbitrary widgets
   // to see if they want to respond to an event
   struct EventHandler {
      EventHandler(Canvas* canvas, const InputEvent& event)
      : canvas(canvas)
      , event(event)
      {}
      ~EventHandler(){
         if (!handler) return;
         switch (event.eventId){
            case InputEventMouseToolTip::ID:
               if (!handler->getToolTipText().empty()) {
                  canvas->setToolTip(handler);
               }
               break;
         }
      }
      EventHandler& operator=(Widget* w){handler = w; return *this;}
      operator bool(){return handler!=nullptr;}
      operator Widget*(){return handler;}
   private:
      Widget* handler = nullptr;
      Canvas* canvas = nullptr;
      const InputEvent& event;
   };

   EventHandler handled(this, event);

   auto createProcessNodeForEvent = [&](TypeNode *thisNode, bool isModal, const InputEvent& event, auto&&... args) -> Widget* {
      if (event.isEvent<InputEventMouseHover>()){
         return processNode<HoverProcess>(thisNode, isModal, event, std::forward<decltype(args)>(args)...);
      }
      return processNode<InputProcess>(thisNode, isModal, event);
   };

   //reject mouse input from outside the canvas
   if (_ignoreOutsideInput) {
      if (isMouse && !getSizeRect().contains(isMouse.value()->getLocalPos() - getPos())){
         return nullptr;
      }
   }

   //query modal widgets first. A modal widget consumes input even if unhandled and prevents anyone else from getting it.
   if (auto modal = getModal()){
      handled = createProcessNodeForEvent(modal->_node, true, event);
      if (modal->_handleAllModalInput || handled) return handled;
   }

   //then focused widgets
   if (auto focus = getFocus()){
      handled = createProcessNodeForEvent(focus->_node, true, event);
      if (handled) return handled;
   }

   //then foreground (which is unaffected by camera transform)
   for (auto& child : _foreground.getValues() | std::views::reverse) {
      handled = createProcessNodeForEvent(child, false, event);
      if (handled) return handled;
   }

   //then background (which is affected by camera transorm)
   // this here is "normal" input
   for (auto& child : _background.getValues() | std::views::reverse) {
      std::unique_ptr<MouseEvent::ScopeTransformer> xformer;
      if (isMouse){
         xformer = make_unique<MouseEvent::ScopeTransformer>(*event.isMouse().value(), getLocalTransform(), child->as<Widget>().value()->size, getCameraTransform());
      }
      handled = createProcessNodeForEvent(child, false, event);
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