#include "Canvas.h"
#include <stack>
#include "rlgl.h"
#include "MiscTools.h"

using namespace std;
using namespace ReyEngine;
using namespace Tools;

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DBegin() {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
   static constexpr char INDENT = '-';
   std::string indent = "";
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
//   size_t doParent = std::numeric_limits<size_t>::max();
//   size_t ihParent = std::numeric_limits<size_t>::max();
//   std::function<std::vector<TypeNode*>(TypeNode*)> searchTree = [&](TypeNode* node){
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
void Canvas::tryRender(TypeNode *node) {
   for (auto& child: node->getChildren()) {
      if (auto isDrawable = child->as<Drawable2D>()) {
         auto& drawable = isDrawable.value();
         rlPushMatrix();
         rlMultMatrixf(MatrixToFloat(drawable->getTransform().matrix));
         drawable->render2DBegin();
         drawable->render2D();
         drawable->render2DEnd();
         tryRender(child);
      }
   }
   rlPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::tryHandle(InputEvent& event, TypeNode* node) {
//   cout << "\"" << node->name << "\"" << " processing input event at parent's pos " << event.isMouse().value()->getLocalPos() << endl;
   //----------------------------------------------------------------------------------------------------
   //lazy initialization and auto-cleanup of mouse transformations. When this falls out of scope, it will
   // revert the local position back to it's parent. But only if it was transformed in the first place.
   // pretty sneaky!
   std::unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;
   //----------------------------------------------------------------------------------------------------

   //first transform the mouse coordinates so that they're relative to the current node
   auto mouseData = event.isMouse();
   if (mouseData) {
      if (auto isPositionable = node->tag<Positionable2D>()) {
         auto& positionable = isPositionable.value();
         mouseTransformer = make_unique<MouseEvent::ScopeTransformer>(*mouseData.value(), positionable->getLocalTransform(), positionable->getSize());
//         cout << "\"" << node->name << "\"" << " transforming to local pos " << mouseTransformer->getLocalPos() << endl;
      }
   }

   // offer the input to children first
   // Note: this always travels the entire tree, before any processing is done
   // so we can use it to keep track of which widget is currently at the mouse cursor
   for (auto& child: node->getChildren()) {
      if (auto isHandler = child->tag<Widget>()) {
         auto handled = tryHandle(event, child);
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
Widget* Canvas::tryHover(InputEventMouseMotion& motion, TypeNode* node) const {
   //see tryHandle for explanation
   std::unique_ptr<MouseEvent::ScopeTransformer> mouseTransformer;

   if (auto isPositionable = node->tag<Positionable2D>()) {
      auto& positionable = isPositionable.value();
      mouseTransformer = make_unique<MouseEvent::ScopeTransformer>(motion.mouse, positionable->getLocalTransform(), positionable->getSize());
   }

   for (auto& child: node->getChildren()) {
      if (auto isWidget = child->tag<Widget>()) {
         auto handled = tryHover(motion, child);
         if (handled) return handled;
      }
   }
   if (auto isWidget = node->as<Widget>()){
      auto widget = isWidget.value();
      if (widget->acceptsHover && motion.mouse.isInside()){
         cout << "The currently hovered widget is " << widget->_node->name << endl;
         return widget;
      }
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::renderProcess() {
   if (!_visible) return;
   ClearBackground(Colors::none);
   rlPushMatrix();
   render2DBegin();

   //front render
   tryRender(_node);

   rlPopMatrix();
   render2DEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2D() const {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DEnd() {

}

/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<float>> Canvas::getMousePos() {
   return InputManager::getMousePos().get();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::__process_hover(const InputEventMouseMotion& event){
   setHover(tryHover(const_cast<InputEventMouseMotion&>(event), _node));
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas::__process_unhandled_input(const InputEvent& event) {
   return tryHandle(const_cast<InputEvent&>(event), _node);
}