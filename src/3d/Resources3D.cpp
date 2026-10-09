#include "Resources3D.h"
#include "rlgl.h"
#include <mutex>

using namespace std;
using namespace ReyEngine;

namespace {
   //see ReyTexture::OrphanedTextureCache
   struct OrphanCache3D {
      static OrphanCache3D& instance(){static OrphanCache3D cache; return cache;}
      std::mutex mtx;
      std::vector<Mesh> meshes;
      std::vector<Model> models;
   };
}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::flushOrphaned3DResources() {
   auto& cache = OrphanCache3D::instance();
   std::scoped_lock lock(cache.mtx);
   for (auto& mesh : cache.meshes) UnloadMesh(mesh);
   for (auto& model : cache.models) UnloadModel(model);
   cache.meshes.clear();
   cache.models.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyMesh::~ReyMesh() {
   if (!_mesh.vertexCount) return;
   auto& cache = OrphanCache3D::instance();
   std::scoped_lock lock(cache.mtx);
   cache.meshes.push_back(_mesh);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ReyMesh> ReyMesh::makeCube(const Size3<float>& size) {
   return make_shared<ReyMesh>(GenMeshCube(size.x, size.y, size.z));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ReyMesh> ReyMesh::makeSphere(float radius, int rings, int slices) {
   return make_shared<ReyMesh>(GenMeshSphere(radius, rings, slices));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ReyMesh> ReyMesh::makePlane(float width, float length, int resX, int resZ) {
   return make_shared<ReyMesh>(GenMeshPlane(width, length, resX, resZ));
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyMaterial::ReyMaterial()
: _material(LoadMaterialDefault())
{}

/////////////////////////////////////////////////////////////////////////////////////////
ReyMaterial::~ReyMaterial() {
   //the shader and textures are shared, so only the map array belongs to us. (UnloadMaterial would free them too.)
   MemFree(_material.maps);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyMaterial::setTexture(const std::shared_ptr<ReyTexture>& texture) {
   _texture = texture;
   _material.maps[MATERIAL_MAP_DIFFUSE].texture = texture ? texture->getTexture() : Texture2D{rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyMaterial::setShader(const std::shared_ptr<ReyShader>& shader) {
   _shader = shader;
   if (shader) {
      _material.shader = shader->getShader();
   } else {
      _material.shader = {rlGetShaderIdDefault(), rlGetShaderLocsDefault()};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyModel::ReyModel(const FileSystem::File& file) {
   if (!file.exists()) {
      Logger::error() << "ReyModel: file does not exist: " << file.abs() << endl;
      return;
   }
   _model = LoadModel(file.canonical().c_str());
   if (!IsModelValid(_model)) Logger::error() << "ReyModel: failed to load " << file.abs() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyModel::~ReyModel() {
   if (!_model.meshCount && !_model.materialCount) return;
   auto& cache = OrphanCache3D::instance();
   std::scoped_lock lock(cache.mtx);
   cache.models.push_back(_model);
}
