#include "BaseBody.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderer3D::renderer3DChain() {
   renderer3DBegin();
   //front render
   for (const auto& child : _childOrder3D){
      child->renderable3DChain();
   }
   renderer3DEnd();
   renderer3DEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderable3D::renderable3DChain(){
   if (!_visible) return;
   render3DBegin();
   render3D();
   for (const auto& child : _childOrder3D){
      child->renderable3DChain();
   }
   render3DEnd();
   renderable3DEditorFeatures();
}