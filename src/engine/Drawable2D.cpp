#include "Drawable2D.h"
#include "rlgl.h"

using namespace ReyEngine;
using namespace Internal;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Drawable2D::_on_child_added_to_tree(TypeNode* n) {}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Drawable2D::render2DChain(){
//   if (!_visible) return;
//   render2DBegin();
//
//   rlPushMatrix();
////   rlTranslatef(transform.position.x, transform.position.y, 0);
////   rlRotatef(transform.rotation * 180/M_PI, 0,0,1);
////   rlScalef(transform.scale.x, transform.scale.y, 1);
//   rlMultMatrixf((const float*)&transform2D.matrix);
//
//   render2D();
//
//   //front render
//   for (const auto &child: children) {
//      child->render2DChain();
//   }
//   rlPopMatrix();
//   render2DEnd();
//   render2DEditorFeatures();
}

