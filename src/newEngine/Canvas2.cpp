#include "Canvas2.h"
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
      cacheDrawables();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas::cacheDrawables() {
   //walk the tree and cache all our drawables in the correct rendering order
   drawOrder.clear();
   std::string indent = "";
   cout << _node->name << endl;
   std::function<std::vector<TypeNode*>(TypeNode*)> getDrawables = [&](TypeNode* node){
      indent += "   ";
      for (const auto& child : node->getChildren()){
         cout << indent << child->name << endl;
         if (auto drawable = child->as<Drawable2D>()){
            drawOrder.emplace_back(drawable.value()->getTransform().getMatrix(), drawable.value());
         }
         getDrawables(child);
      }
      indent.resize(indent.length() - 3);
      return node->getChildren();
   };

   auto parent = _node->getParent();
   auto root = _node->getRoot();

   getDrawables(_node);
   std::reverse(drawOrder.begin(), drawOrder.end());
   cout << "New Draw order is: " << endl;
   for (const auto& [Matrix, drawable] : drawOrder){
      cout << drawable->getNode()->getScenePath() << ",";
   }
   cout << endl;
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
   for (const auto& pair: drawOrder) {
      auto& drawable = pair.second;
      if (!drawable->_visible) continue;
      rlPushMatrix();
      rlMultMatrixf((const float*)&pair.first);
      drawable->render2DBegin();
      drawable->render2D();
      drawable->render2DEnd();
      rlPopMatrix();
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