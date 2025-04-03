#include "Canvas.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;
using namespace Tools;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2D() const {
   Logger::debug() << "Rendering canvas " << getName() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   _renderTarget.setSize(getSize());
//   _viewport = getSizeRect();
//   _projectionPort = getSizeRect();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
   static constexpr char INDENT = '-';
   string indent = "";
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
Widget* Canvas::tryHandle(InputEvent& event, TypeNode* node, const Transform2D& inputTransform) {
//   cout << "\"" << node->name << "\"" << " processing input event at parent's pos " << event.isMouse().value()->getLocalPos() << endl;
   //----------------------------------------------------------------------------------------------------
   //lazy initialization and auto-cleanup of mouse transformations. When this falls out of scope, it will
   // revert the local position back to it's parent. But only if it was transformed in the first place.
   // pretty sneaky!
   unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;
   //----------------------------------------------------------------------------------------------------

   //first transform the mouse coordinates so that they're relative to the current node
   auto mouseData = event.isMouse();
   if (mouseData) {
      if (auto isCanvas = node->as<Canvas>()){

      }
      if (auto isPositionable = node->tag<Positionable2D>()) {
         auto& positionable = isPositionable.value();
         mouseTransformer = make_unique<MouseEvent::ScopeTransformer>(*mouseData.value(), inputTransform, positionable->getSize());
//         cout << "\"" << node->name << "\"" << " transforming to local pos " << mouseTransformer->getLocalPos() << endl;
      }
   }

   // then offer to children
   // Note: this return cascades when the first widget accepts the event.
   for (auto& child: node->getChildren()) {
      if (auto isHandler = child->tag<Widget>()) {
         auto handled = tryHandle(event, child, isHandler.value()->getTransform());
         if (handled) return handled;
      }
   }
   //if no children want this event, we try to process it ourselves
   // need to take care that we don't enter a recursive loop when the canvas that originated the event
   // tries to handle it
   if (auto iswidget = node->as<Widget>()){
      if (auto thisCanvas = node->as<Canvas>()) {
         if (thisCanvas == this) {
            return _unhandled_input(event);
         }
      }
      return iswidget.value()->__process_unhandled_input(event);
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::tryHover(InputEventMouseMotion& motion, TypeNode* node, const Transform2D& inputTransform) const {
   //see tryHandle for explanation
   unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;

   if (auto isPositionable = node->tag<Positionable2D>()) {
      auto& positionable = isPositionable.value();
      mouseTransformer = make_unique<MouseEvent::ScopeTransformer>(motion.mouse, inputTransform, positionable->getSize());
   }

   for (auto& child: node->getChildren()) {
      if (auto isWidget = child->tag<Widget>()) {
         auto handled = tryHover(motion, child, isWidget.value()->getLocalTransform());
         if (handled) return handled;
      }
   }
   if (auto isWidget = node->as<Widget>()){
      auto widget = isWidget.value();
      if (widget->acceptsHover && motion.mouse.isInside()){
         Logger::debug() << "The currently hovered widget is " << widget->_node->name << endl;
         return widget;
      }
   }
   return nullptr;
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
      transformStack.pushTransform(&modalXform);

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
void Canvas::__process_hover(const InputEventMouseMotion& event){
   //modal widgets eat input
   auto& event_non_const = const_cast<InputEventMouseMotion&>(event);
   if (!_intrinsicChildren.empty()){

   }
   if (auto modal = getModal()){
      setHover(tryHover(event_non_const, modal->_node, modalXform));
   } else {
      setHover(tryHover(const_cast<InputEventMouseMotion&>(event), _node, getLocalTransform()));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* Canvas::__process_unhandled_input(const InputEvent& event) {
   //query intrinsic children first
   for (auto& intrinsicChild : _intrinsicChildren) {
      auto handled = processNode<InputProcess>(intrinsicChild.get(), false, event);
      if (handled) return handled;
   }

   //reject canvas input from outside the canvas
   if (_rejectOutsideInput) {
      if (auto isMouse = event.isMouse()) {
         auto mouseTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), getLocalTransform(), getSize());
         if (!event.isMouse().value()->isInside()) {
            return nullptr;
         }
      }
   }


   //then modal widgets
   if (auto modal = getModal()){
      return processNode<InputProcess>(modal->_node, true, event);
   }

   auto cameraTransformer = MouseEvent::ScopeTransformer(*event.isMouse().value(), MatrixIdentity(), getSize(), GetCameraMatrix2D(camera));
   return processNode<InputProcess>(_node, false, event);
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