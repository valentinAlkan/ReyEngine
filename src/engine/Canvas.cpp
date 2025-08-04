#include "Canvas.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

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
   ReyObject::__on_child_added_to_tree(n);
}


////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   _renderTarget.setSize(getSize());
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
      ClearBackground(Colors::none);
      drawRectangleGradientV(getRect().toSizeRect(), Colors::green, Colors::yellow);
      drawText(getName(), {0,0}, theme->font);
   }

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
   }

   rlPopMatrix();

   //root canvas has no parent canvas. So ensure root canvas draws its foreground.
   rlPushMatrix();
   if (!getCanvas()) {
      for (auto& child : _foreground.getValues()){
         processNode<RenderProcess>(child, false);
      }
   }
   rlPopMatrix();

   render2DEnd();
   _renderTarget.endRenderMode();
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
   auto _event = const_cast<InputEventMouseHover&>(event); //should not be rvalue
   auto accepted = __process_unhandled_input(_event);
   setHover(accepted);
   return accepted;
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::__process_unhandled_input(const InputEvent& event) {
   auto isMouse = event.isMouse();
   //determine what type of input this is

   auto createProcessNodeForEvent = [&](TypeNode *thisNode, bool isModal, const InputEvent& event, auto&&... args){
      if (event.isEvent<InputEventMouseHover>()){
         return processNode<HoverProcess>(thisNode, isModal, event, std::forward<decltype(args)>(args)...);
      }
      return processNode<InputProcess>(thisNode, isModal, event);
   };

   //reject mouse input from outside the canvas
   if (_rejectOutsideInput) {
      if (isMouse) {
         auto mouseTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), getLocalTransform(), getSize());
         if (!event.isMouse().value()->isInside()) {
            return nullptr;
         }
      }
   }

   Widget* handled = nullptr;
   //query modal widgets first. A modal widget consumes input even if unhandled.
   if (auto modal = getModal()){
      return createProcessNodeForEvent(modal->_node, true, event);
   }

   //then focused widgets
   if (auto focus = getFocus()){
      handled = createProcessNodeForEvent(focus->_node, true, event);
      if (handled) return handled;
   }

   //then foreground
   for (auto& child : _foreground.getValues()) {
      handled = createProcessNodeForEvent(child, false, event);
      if (handled) return handled;
   }

   //then background - which is affected by camera
   if (isMouse) {
      auto cameraTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), Transform2D(), getSize(), getCameraTransform());
      handled = createProcessNodeForEvent(_node, false, event);
   } else {
      //propagate to children
       handled = createProcessNodeForEvent(_node, false, event);
   }

   return handled;
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
      Logger::error() << "Canvas: " << fxName << " - widget must be a child of this canvas at this time" << endl;
      return {};
   } else if (node->getParent() != canvas->getNode()){
      Logger::error() << "Canvas: " << fxName << " - widget must be a child of this canvas" << endl;
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