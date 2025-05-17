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
void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
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

//   for (auto& child : _foreground.getValues()){
//      processNode<RenderProcess>(child, false);
//   }

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
void Canvas::moveToForeground(Widget* widget) {
   if (!widget) {
      Logger::error() << "Canvas: moveToForeground - Null widget!" << endl;
      return;
   }

   auto node = widget->getNode();
   if (!widget->getNode()) {
      Logger::error() << "Canvas: moveToForeground - Null node!" << endl;
      return;
   }
   _background.remove(node);
   _foreground.add(node);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::moveToBackground(Widget* widget) {
   if (!widget) {
      Logger::error() << "Canvas: moveToForeground - Null widget!" << endl;
      return;
   }

   auto node = widget->getNode();
   if (!node) {
      Logger::error() << "Canvas: moveToBackground - Null node!" << endl;
      return;
   }
   _foreground.remove(node);
   _background.add(node);
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