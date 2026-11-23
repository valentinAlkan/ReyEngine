#pragma once
#include <memory>
#include <vector>
#include "ReyEngine3D.h"
#include "Shader.h"

// Owning wrappers around raylib's 3D resources, in the same spirit as ReyTexture/ReyImage/ReyShader.
// They are non-copyable and meant to be shared through std::shared_ptr, so many nodes can draw the same mesh.
namespace ReyEngine {
   /// A mesh's vertices, kept in plain vectors: it can be built on any thread and frees itself, and ReyMesh::upload sends
   /// it to the GPU. Every vertex stands alone (no index buffer), three to a triangle, wound counterclockwise as seen from
   /// the front. normals, colors and texcoords are each either empty or one per vertex.
   struct MeshData {
      std::vector<Vec3<float>> positions;
      std::vector<Vec3<float>> normals;
      std::vector<ColorRGBA> colors;
      std::vector<Vec2<float>> texcoords; //zeros are uploaded if empty
      [[nodiscard]] size_t vertexCount() const {return positions.size();}
      [[nodiscard]] bool empty() const {return positions.empty();}
      void reserve(size_t vertices);
      /// adds other's vertices after these. Both must have the same parts (normals, colors, texcoords) filled in.
      void append(const MeshData& other);
      /// the smallest box holding every vertex (empty if there are none)
      [[nodiscard]] Box<float> getBoundingBox() const;
      bool operator==(const MeshData&) const = default;
   };

   /// Owns a raylib Mesh. GPU buffers can only be freed by the main thread, so (like ReyTexture) a released mesh is
   /// handed to an orphan cache which is emptied once per frame by Canvas3D.
   class ReyMesh {
   public:
      explicit ReyMesh(const Mesh& mesh); //takes ownership. The bounding box is worked out now, from the CPU copy if it has one
      /// Uploads the vertices to the GPU: main thread only, once a window exists. The CPU copy is let go unless keepCpuCopy,
      /// but the bounding box is kept either way. Without the CPU copy the mesh can only be picked against its bounding box.
      static std::shared_ptr<ReyMesh> upload(const MeshData& data, bool keepCpuCopy = false);
      ReyMesh(const ReyMesh&) = delete;
      ReyMesh& operator=(const ReyMesh&) = delete;
      ~ReyMesh();
      //unit primitives. These upload to the GPU, so they must be called from the main thread once a window exists.
      static std::shared_ptr<ReyMesh> makeCube(const Size3<float>& size = Size3<float>(1));
      static std::shared_ptr<ReyMesh> makeSphere(float radius = 1, int rings = 32, int slices = 32);
      static std::shared_ptr<ReyMesh> makePlane(float width = 1, float length = 1, int resX = 1, int resZ = 1);
      //unlike the cube and sphere, which are centred on the origin, the cylinder stands on its base at the origin and
      // reaches up the +Y axis to height (as raylib's GenMeshCylinder makes it). It is capped at both ends.
      static std::shared_ptr<ReyMesh> makeCylinder(float radius = 1, float height = 1, int slices = 32);
      [[nodiscard]] const Mesh& getMesh() const {return _mesh;}
      [[nodiscard]] Box<float> getBoundingBox() const {return _bounds;}
      [[nodiscard]] bool hasCpuCopy() const {return _mesh.vertices != nullptr;}
   private:
      ReyMesh(const Mesh& mesh, const Box<float>& bounds): _mesh(mesh), _bounds(bounds){}
      Mesh _mesh = {};
      Box<float> _bounds;
   };

   /// A raylib Material whose shader and diffuse texture are shared rather than owned, so releasing the material
   /// never frees a shader or texture someone else is still using.
   class ReyMaterial {
   public:
      ReyMaterial();
      explicit ReyMaterial(const ColorRGBA& color): ReyMaterial(){setColor(color);}
      ReyMaterial(const ReyMaterial&) = delete;
      ReyMaterial& operator=(const ReyMaterial&) = delete;
      ~ReyMaterial();
      void setColor(const ColorRGBA& color){_material.maps[MATERIAL_MAP_DIFFUSE].color = (Color)color;}
      [[nodiscard]] ColorRGBA getColor() const {return _material.maps[MATERIAL_MAP_DIFFUSE].color;}
      void setTexture(const std::shared_ptr<ReyTexture>& texture);
      [[nodiscard]] const std::shared_ptr<ReyTexture>& getTexture() const {return _texture;}
      //null restores raylib's default shader
      void setShader(const std::shared_ptr<ReyShader>& shader);
      [[nodiscard]] const std::shared_ptr<ReyShader>& getShader() const {return _shader;}
      [[nodiscard]] const Material& getMaterial() const {return _material;}
   private:
      Material _material = {};
      std::shared_ptr<ReyShader> _shader;
      std::shared_ptr<ReyTexture> _texture;
   };

   /// Owns a raylib Model (its meshes and materials) loaded from disk.
   class ReyModel {
   public:
      explicit ReyModel(const FileSystem::File& file);
      ReyModel(const ReyModel&) = delete;
      ReyModel& operator=(const ReyModel&) = delete;
      ~ReyModel();
      [[nodiscard]] explicit operator bool() const {return IsModelValid(_model);}
      [[nodiscard]] const Model& getModel() const {return _model;}
      [[nodiscard]] Box<float> getBoundingBox() const {return Box<float>(GetModelBoundingBox(_model));}
   private:
      Model _model = {};
   };

   namespace Internal {
      //Unloads meshes and models released since the last call. Must be called from the main thread.
      void flushOrphaned3DResources();
   }
}
