#include "MeshInstance3D.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void MeshInstance3D::render3D(RenderContext3D& context) const {
   if (!_mesh) return;
   if (_material) {
      context.drawMesh(*_mesh, *_material);
   } else {
      static const ReyMaterial defaultMaterial;
      context.drawMesh(*_mesh, defaultMaterial);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<R_FLOAT> MeshInstance3D::intersectRay(const Ray3D& sceneRay) const {
   //cheap bounds test first
   if (!Node3D::intersectRay(sceneRay)) return {};
   auto collision = GetRayCollisionMesh(sceneRay, _mesh->getMesh(), getGlobalTransform().matrix);
   if (collision.hit) return collision.distance;
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<R_FLOAT> ModelInstance3D::intersectRay(const Ray3D& sceneRay) const {
   if (!Node3D::intersectRay(sceneRay)) return {};
   const auto& model = _model->getModel();
   auto xform = MatrixMultiply(model.transform, getGlobalTransform().matrix);
   std::optional<R_FLOAT> nearest;
   for (int i = 0; i < model.meshCount; i++) {
      auto collision = GetRayCollisionMesh(sceneRay, model.meshes[i], xform);
      if (collision.hit && (!nearest || collision.distance < nearest.value())) nearest = collision.distance;
   }
   return nearest;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ModelInstance3D::render3D(RenderContext3D& context) const {
   if (_model && *_model) context.drawModel(*_model, _tint);
}
