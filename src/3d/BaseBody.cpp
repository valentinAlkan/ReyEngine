#include "BaseBody.h"
#include "Logger.h"
#include "Application.h"
#include "TypeContainer.h"
#include "Viewport.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

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
void Internal::Renderable3D::render3D() const {
    if (_model) {
        DrawCube({5,0,0}, 1,1,1, Colors::blue);
        DrawModel(_model->getModel(), _transform.position, _scale, _tint.value);
    }
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
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Collisions::RayHit3D> Internal::Renderer3D::testRayClosest(const Collisions::Ray3D& ray){
    std::optional<Collisions::RayHit3D> hit;
    for (const auto& child : getChildren()){
        auto closestChildHit = child->testRayClosest(ray);
        if (closestChildHit && closestChildHit) {
            //found a closer model
            if (hit && closestChildHit.value().distance < closestChildHit.value().distance) {
                hit = closestChildHit;
                continue;
            }
            hit = closestChildHit;
        }
    }
    return hit;
}

///////////////////////////////////////////////////////////////////////////////////////////
template <>
void ReyEngine::Internal::TypeContainer<Renderable3D>::__on_child_added_immediate(std::shared_ptr<Renderable3D>& child){
   child->__init();
   _on_child_added(child);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Collisions::RayHit3D> Internal::Renderable3D::testRayClosest(const Collisions::Ray3D& ray) {
    //call the parent testray function
    auto _hit = Renderer3D::testRayClosest(ray);
    //actually test the ray against ourselves
    if (!_visible) return _hit;
    for (const auto &mesh: _model->getMeshes()){
        auto ourHit = Collisions::collides(ray, mesh);
        if (!_hit && ourHit || ourHit && _hit->distance > ourHit->distance){
            _hit = ourHit;
        }
    }
    return _hit;
}