#include "Canvas.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;
using namespace Tools;
using namespace Internal;
/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DBegin() {
   //save the global matrix transform off
//   globalXform = getGlobalTransform();
//   rlPushMatrix();
//   rlLoadIdentity();
//   getRenderTarget()->beginRenderMode();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DEnd() {
//   rlMultMatrixf(MatrixToFloat(globalXform.matrix));
//   rlPopMatrix();
//   getRenderTarget()->endRenderMode();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2D() const {
   Logger::debug() << "Rendering canvas " << getName() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_rect_changed() {
   renderTarget.setSize(getSize());
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
void Canvas::cacheTree(size_t drawOrderSize, size_t inputOrderSize) {
   //walk the tree and cache all our drawables in the correct rendering order
//   drawOrder.clear();
//   drawOrder.reserve(drawOrderSize); //keep the vector from reallocating
//   inputOrder.clear();
//   inputOrder.reserve(inputOrderSize); //keep the vector from reallocating
//
//   cout << _node->name << endl;
//   size_t doParent = numeric_limits<size_t>::max();
//   size_t ihParent = numeric_limits<size_t>::max();
//   function<vector<TypeNode*>(TypeNode*)> searchTree = [&](TypeNode* node){
//
//      //update global transform to account for this nodes transform, if applicable
//      for (auto& child : node->getChildren()){
//         cout << indent << child->name << endl;
//         if (auto isDrawable = child->as<Drawable2D>()){
//            auto& drawable = isDrawable.value();
//            drawOrder.emplace_back(drawable, node, drawOrder.size(), );
//         }
//         if (auto isHandler = child->tag<InputHandler>()){
//            auto& handler = isHandler.value();
//            inputOrder.emplace_back(handler, ihParent, child);
//         }
//         //do not cache the children of canvases
//         if (child->as<Canvas>()) continue;
//         doParent =
//      }
//      indent.resize(indent.length() - 3);
//      return node->getChildren();
//   };
//
//   searchTree(_node);
//   cout << "New Draw order is: " << endl;
//   for (auto& orderable : drawOrder){
//      cout << orderable.data->getNode()->getScenePath() << ",";
//   }
//
//   cout << "New input order is: " << endl;
//   for (auto& orderable : inputOrder){
//
//   }
//   cout << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::updateGlobalTransforms() {
//   for (auto& drawable : drawOrder){
////      drawable.second
//   }
}

/////////////////////////////////////////////////////////////////////////////////////////
//void Canvas::tryRender(TypeNode *thisNode, optional<Drawable2D*> isSelfDrawable, bool drawModal) {
//   //ignore invisible
//   if (isSelfDrawable && !isSelfDrawable.value()->_visible) return;
//
//   //draws actual drawable itself
//   if (isSelfDrawable){
//      Transformer transformer(*this, isSelfDrawable);
//      auto& drawable = isSelfDrawable.value();
//      if (drawable->_isCanvas){
//         auto canvas = thisNode->as<Canvas>().value();
//         if (canvas != this) {
//            //subcanvas
//            rlPopMatrix();
//            getRenderTarget()->endRenderMode();
//            canvas->renderProcess();
//            getRenderTarget()->beginRenderMode();
//            rlPushMatrix();
//            rlMultMatrixf(MatrixToFloat(transformStack.getGloablTransform().matrix));
//            auto _renderTarget = canvas->getRenderTarget();
//            DrawTextureRec(_renderTarget->getRenderTexture(), {0, 0, (float) _renderTarget->getSize().x, -(float) getSize().y}, {0, 0}, WHITE);
//            //does not dispatch draw to children as this is done by renderProcess
//         }
//      } else {
//         //normal render with child dispatch
//         if (!drawable->_modal || drawModal) {
//            drawable->render2DBegin();
//            drawable->render2D();
//            drawable->render2DEnd();
//         }
//         tryRenderChildren(thisNode, drawModal);
//      }
//      // go no further
//      return;
//   }
//
//   //non-drawable node with children - drawables should never reach this
//   tryRenderChildren(thisNode, drawModal);
//}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::tryRenderChildren(TypeNode *thisNode, bool drawModal){
   //dispatch to children
   for (auto& child: thisNode->getChildren()) {
      if (auto childDrawable = child->as<Drawable2D>()){
         auto& drawable = childDrawable.value();
         if (drawable->_modal) {
            //save off global transformation matrix so we can redraw this widget
            // later in its proper position
            // Note: This encodes the drawables local transform, which needs to be subtracted off later.
            modalXform = drawable->getGlobalTransform();
         } else {
            tryRender<RenderProcess>(child, childDrawable, false);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::tryHandle(InputEvent& event, TypeNode* node, const Transform2D& inputTransform) {
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
         if (handled) { return true; }
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
   return false;
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
   getRenderTarget()->beginRenderMode();
   drawRectangleGradientV(getRect().toSizeRect(), Colors::green, Colors::yellow);
   rlPushMatrix();
   //front render - first pass, don't draw modal
   tryRenderChildren(_node, static_cast<Drawable2D*>(this));

   //the modal widget's xform includes canvas xform, so we want to pop that off as if
   // we are rendering globally
   if (auto modal = getModal()){
      auto modalDrawable = modal->_node->as<Drawable2D>();
      transformStack.pushTransform(&modalXform);

      //invert (subtract off) the modal widget's own position since it's already encoded in modalXform.
      auto inverseXform = modalDrawable.value()->getTransform().inverse();
      transformStack.pushTransform(&inverseXform);

//      Logger::debug() << "Drawing " << modal->_node->getName() << " at " << modalXform.extractTranslation() + _node->as<Drawable2D>().value()->getPosition() << endl;
      tryRender<RenderProcess>(modal->_node, modalDrawable, true);
      transformStack.popTransform();
   }

   rlPopMatrix();
   getRenderTarget()->endRenderMode();
}

/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<float>> Canvas::getMousePos() {
   return InputManager::getMousePos().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__process_hover(const InputEventMouseMotion& event){
   //modal widgets eat input
   auto& event_non_const = const_cast<InputEventMouseMotion&>(event);
   if (auto modal = getModal()){
      setHover(tryHover(event_non_const, modal->_node, modalXform));
   } else {
      setHover(tryHover(const_cast<InputEventMouseMotion&>(event), _node, getLocalTransform()));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::__process_unhandled_input(const InputEvent& event) {
   //modal widgets eat input
   auto& event_non_const = const_cast<InputEvent&>(event);
   if (auto modal = getModal()){
      return tryHandle(event_non_const, modal->_node, modalXform);
   }

   return tryHandle(event_non_const, _node, getLocalTransform());
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