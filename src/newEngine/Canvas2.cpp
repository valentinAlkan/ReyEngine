#include "Canvas2.h"
#include <stack>
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::render2DBegin() {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::_on_descendant_added_to_tree(TypeNode *n) {
   if (auto drawable = n->as<Drawable2D>()) {
      std::cout << "Descendant " << n->name << " is drawable" << std::endl;
      cacheDrawables(drawOrder.size()+1);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::cacheDrawables(size_t count) {
   //walk the tree and cache all our drawables in the correct rendering order
   drawOrder.clear();
   drawOrder.reserve(count); //keep the vector from reallocating
   std::string indent = "";
   cout << _node->name << endl;
   std::function<std::vector<TypeNode*>(TypeNode*, Matrix&)> getDrawables = [&](TypeNode* node, Matrix& globalTransform){
      Matrix _globalTransform;
      indent += "   ";
      //update global transform to account for this nodes transform, if applicable
      if (auto isPositionable = node->tag<Internal::Positionable2D2>()){
         _globalTransform = MatrixMultiply(globalTransform, isPositionable.value()->getLocalTransform().matrix);
      } else {
         _globalTransform = globalTransform;
      }
      for (auto& child : node->getChildren()){
         cout << indent << child->name << endl;
         if (auto isDrawable = child->as<Drawable2D>()){
            auto& drawable = isDrawable.value();
            Matrix childGlobalMatrix = MatrixMultiply(_globalTransform, drawable->transform2D.matrix);
            drawable->getGlobalTransform().matrix = childGlobalMatrix;
            drawOrder.emplace_back(drawable);
            drawOrder.back().childCount = child->getChildren().size();
         }
         getDrawables(child, _globalTransform);
      }
      indent.resize(indent.length() - 3);
      return node->getChildren();
   };

   Matrix globalTransform = MatrixIdentity();
   getDrawables(_node, globalTransform);
   cout << "New Draw order is: " << endl;
   for (auto& data : drawOrder){
      cout << data.drawable->getNode()->getScenePath() << ",";
   }
   cout << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::updateGlobalTransforms() {
   for (auto& drawable : drawOrder){
//      drawable.second
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::renderProcess() {
   ClearBackground(Colors::none);
   if (!_visible) return;
   render2DBegin();

   rlPushMatrix();
   rlMultMatrixf((const float*)&transform2D.matrix);

   render2D();

   //front render
   //iterate backwards since we are looking at pointers to vector entries that would be invalidated if reversed
   std::stack<size_t> pushStack;
   for (auto& data : drawOrder) {
      auto& drawable = data.drawable;
      pushStack.push(data.childCount);
      cout << "Drawing drawable " << drawable->getNode()->name << endl;
      cout << "    (child count = " << data.childCount << ")" << endl;
      cout << "    (pushStack = " << pushStack.top() << ")" << endl;
      if (!drawable->_visible) continue;
      rlPushMatrix();
      rlMultMatrixf(MatrixToFloat(data.drawable->getTransform().matrix));
      drawable->render2DBegin();
      drawable->render2D();
      drawable->render2DEnd();
      while (!pushStack.empty() && pushStack.top() == 0) {
         rlPopMatrix();
         pushStack.pop();
         if (pushStack.empty()) break;
         pushStack.top() -= 1;
      }
   }
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

}