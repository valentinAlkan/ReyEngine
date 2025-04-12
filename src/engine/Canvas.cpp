#include "Canvas.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   _renderTarget.setSize(getSize());
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
   static constexpr char INDENT = '-';
   string indent;
   cout << "------------------------" << endl;
   function<void(TypeNode*)> catTree = [&](TypeNode* n){
      cout << indent << n->name << "" << endl;
      indent += INDENT;
      for (const auto& child : n->getChildren()){
         catTree(child);
      }
      indent.resize(indent.length() - sizeof(INDENT)/sizeof(char));
   };
   catTree(_node);
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
void Canvas::renderProcess() {
   if (!_visible) return;
   _renderTarget.beginRenderMode();
   rlPushMatrix();
   render2DBegin();
   BeginMode2D(camera);
   ClearBackground(Colors::none);
   drawRectangleGradientV(getRect().toSizeRect(), Colors::green, Colors::yellow);
   drawText(getName(), {0,0}, theme->font);

   if (_intrinsicRenderType == IntrinsicRenderType::CanvasUnderlay){
      for (auto& intrinsicChild : _intrinsicChildren){
         processNode<RenderProcess>(intrinsicChild.get(), false);
      }
   }

   //normal front render - first pass, don't draw modal
   processChildren<RenderProcess>(_node);

   if (_intrinsicRenderType == IntrinsicRenderType::CanvasOverlay){
      for (auto& intrinsicChild : _intrinsicChildren){
         processNode<RenderProcess>(intrinsicChild.get(), false);
      }
   }

   //the modal widget's xform includes canvas xform, so we want to pop that off as if
   // we are rendering globally
   if (auto modal = getModal()){
      auto modalDrawable = modal->_node->as<Widget>();
      transformStack.pushTransform(&getModal()->getGlobalTransform().get());

      //invert (subtract off) the modal widget's own position since it's already encoded in modalXform.
      auto inverseXform = modalDrawable.value()->getTransform().inverse();
      transformStack.pushTransform(&inverseXform);

      // Logger::debug() << "Drawing " << modal->_node->getName() << " at " << modalXform.extractTranslation() + _node->as<Drawable2D>().value()->getPosition() << endl;
      processNode<RenderProcess>(modal->_node, true);
      transformStack.popTransform();
   }
   EndMode2D();
   rlPopMatrix();
   render2DEnd();
   _renderTarget.endRenderMode();
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

   //query intrinsic children first
   for (auto& intrinsicChild : _intrinsicChildren) {
      auto handled = createProcessNodeForEvent(intrinsicChild.get(), false, event);
      if (handled) return handled;
   }

   //reject canvas input from outside the canvas
   if (_rejectOutsideInput) {
      if (isMouse) {
         auto mouseTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), getLocalTransform(), getSize());
         if (!event.isMouse().value()->isInside()) {
            return nullptr;
         }
      }
   }

   //then modal widgets
   if (auto modal = getModal()){
      return createProcessNodeForEvent(modal->_node, true, event);
   }

   //then focused widgets
   if (auto focus = getFocus()){
      return createProcessNodeForEvent(focus->_node, true, event);
   }

   if (isMouse) {
      auto cameraTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), Transform2D(), getSize(), getCameraTransform());
      return createProcessNodeForEvent(_node, false, event);
   }
   return createProcessNodeForEvent(_node, false, event);
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