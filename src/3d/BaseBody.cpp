#include "BaseBody.h"
#include "Logger.h"
#include "Application.h"
#include "TypeContainer.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
template <>
void TypeContainer<BaseBody>::_on_child_added_immediate(ChildPtr &) {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderer3D::renderer3DChain() {
   renderer3DBegin();
   //front render
   for (const auto& child : getChildren()){
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
   for (const auto& child : getChildren()){
      child->renderable3DChain();
   }
   render3DEnd();
   renderable3DEditorFeatures();
}
