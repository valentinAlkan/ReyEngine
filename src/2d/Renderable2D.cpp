#include "Renderable2D.h"
#include "rlgl.h"
using namespace std;
using namespace ReyEngine;
using namespace Internal;


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderer2D::renderer2DChain() {
   renderer2DBegin();
   //front render - default for now
   for (const auto& child : getChildren()){
      child->renderable2DChain();
   }
   renderer2DEnd();
   renderer2DEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderable2D::renderable2DChain(){
   if (!_visible) return;
   render2DBegin();

   rlPushMatrix();
   rlTranslatef(transform.position.x, transform.position.y, 0);
   rlRotatef(transform.rotation * 180/M_PI, 0,0,1);
   rlScalef(transform.scale.x, transform.scale.y, 1);

   render2D();

   //front render
   for (const auto &child: getChildren()) {
      child->renderable2DChain();
   }
   rlPopMatrix();
   render2DEnd();
   renderable2DEditorFeatures();
}