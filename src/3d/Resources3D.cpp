#include "Resources3D.h"
#include "rlgl.h"
#include <mutex>
#include <cstring>
#include <stdexcept>
#include <algorithm>

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

// MeshData's vectors go to the GPU as raylib's flat arrays, so each element must be laid out exactly as those are
static_assert(sizeof(Vec3<float>) == 3 * sizeof(float) && alignof(Vec3<float>) == alignof(float), "Vec3<float> must be three packed floats");
static_assert(sizeof(Vec2<float>) == 2 * sizeof(float) && alignof(Vec2<float>) == alignof(float), "Vec2<float> must be two packed floats");
static_assert(sizeof(ColorRGBA) == 4 && alignof(ColorRGBA) == 1, "ColorRGBA must be four packed bytes");
static_assert(std::is_standard_layout_v<Vec3<float>> && std::is_standard_layout_v<Vec2<float>> && std::is_standard_layout_v<ColorRGBA>);

/////////////////////////////////////////////////////////////////////////////////////////
void MeshData::reserve(size_t vertices) {
   positions.reserve(vertices);
   normals.reserve(vertices);
   colors.reserve(vertices);
}

/////////////////////////////////////////////////////////////////////////////////////////
void MeshData::append(const MeshData& other) {
   if (other.empty()) return;
   if (empty()) {
      *this = other;
      return;
   }
   if (normals.empty() != other.normals.empty() || colors.empty() != other.colors.empty() || texcoords.empty() != other.texcoords.empty()) {
      throw std::invalid_argument("MeshData::append: both must have the same parts filled in");
   }
   positions.insert(positions.end(), other.positions.begin(), other.positions.end());
   normals.insert(normals.end(), other.normals.begin(), other.normals.end());
   colors.insert(colors.end(), other.colors.begin(), other.colors.end());
   texcoords.insert(texcoords.end(), other.texcoords.begin(), other.texcoords.end());
}

/////////////////////////////////////////////////////////////////////////////////////////
Box<float> MeshData::getBoundingBox() const {
   if (positions.empty()) return {};
   Vec3<float> low = positions.front(), high = positions.front();
   for (const auto& p : positions) {
      low = {std::min(low.x, p.x), std::min(low.y, p.y), std::min(low.z, p.z)};
      high = {std::max(high.x, p.x), std::max(high.y, p.y), std::max(high.z, p.z)};
   }
   return Box<float>(Pos3<float>(low), Size3<float>(high - low));
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyMesh::ReyMesh(const Mesh& mesh)
: _mesh(mesh)
, _bounds(mesh.vertices ? Box<float>(GetMeshBoundingBox(mesh)) : Box<float>())
{}

/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ReyMesh> ReyMesh::upload(const MeshData& data, bool keepCpuCopy) {
   const size_t n = data.vertexCount();
   auto check = [n](size_t size, const char* part){
      if (size && size != n) throw std::invalid_argument(std::string("ReyMesh::upload: ") + part + " must be empty or one per vertex");
   };
   check(data.normals.size(), "normals");
   check(data.colors.size(), "colors");
   check(data.texcoords.size(), "texcoords");
   if (n % 3) throw std::invalid_argument("ReyMesh::upload: the vertices must come three to a triangle");
   if (n == 0) return std::shared_ptr<ReyMesh>(new ReyMesh(Mesh{}, Box<float>()));

   //raylib frees a mesh's CPU arrays itself (UnloadMesh), so a copy that's kept must be in memory it allocated
   auto array = [keepCpuCopy](const void* source, size_t bytes) -> void* {
      if (!keepCpuCopy || !source) return const_cast<void*>(source);
      void* copy = MemAlloc((unsigned int)bytes);
      std::memcpy(copy, source, bytes);
      return copy;
   };
   const std::vector<Vec2<float>> zeros(data.texcoords.empty() ? n : 0);
   const auto& texcoords = data.texcoords.empty() ? zeros : data.texcoords;
   Mesh mesh = {};
   mesh.vertexCount = (int)n;
   mesh.triangleCount = (int)(n / 3);
   mesh.vertices = (float*)array(data.positions.data(), n * sizeof(Vec3<float>));
   mesh.normals = data.normals.empty() ? nullptr : (float*)array(data.normals.data(), n * sizeof(Vec3<float>));
   mesh.colors = data.colors.empty() ? nullptr : (unsigned char*)array(data.colors.data(), n * sizeof(ColorRGBA));
   mesh.texcoords = (float*)array(texcoords.data(), n * sizeof(Vec2<float>));
   UploadMesh(&mesh, false);
   if (!keepCpuCopy) { //they point into data: forget them, so nothing frees them
      mesh.vertices = mesh.normals = mesh.texcoords = nullptr;
      mesh.colors = nullptr;
   }
   return std::shared_ptr<ReyMesh>(new ReyMesh(mesh, data.getBoundingBox()));
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
std::shared_ptr<ReyMesh> ReyMesh::makeCylinder(float radius, float height, int slices) {
   return make_shared<ReyMesh>(GenMeshCylinder(radius, height, slices));
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
