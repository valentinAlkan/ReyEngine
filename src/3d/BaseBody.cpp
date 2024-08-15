#include "BaseBody.h"
#include "Logger.h"
#include "Application.h"
#include "TypeContainer.h"
#include "Viewport.h"

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


///////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Viewport>> Renderer3D::getViewport() {
   auto weakPrt = getParent();
   while (!weakPrt.expired()) {
      auto parent = weakPrt.lock();
      if (!parent) {
         return nullopt;
      }
      if (parent->_get_static_constexpr_typename() == ReyEngine::Viewport::TYPE_NAME) {
         return parent->getViewport();
      }
      weakPrt = parent->Renderer3D::getParent();
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
template <>
void ReyEngine::Internal::TypeContainer<Renderable3D>::__on_child_added_immediate(std::shared_ptr<Renderable3D>& child){
   child->__init();
   _on_child_added(child);
}