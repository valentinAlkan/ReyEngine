#pragma once
#include <memory>
#include "ReyEngine3D.h"
#include "Shader.h"

// Owning wrappers around raylib's 3D resources, in the same spirit as ReyTexture/ReyImage/ReyShader.
// They are non-copyable and meant to be shared through std::shared_ptr, so many nodes can draw the same mesh.
namespace ReyEngine {
   /// Owns a raylib Mesh. GPU buffers can only be freed by the main thread, so (like ReyTexture) a released mesh is
   /// handed to an orphan cache which is emptied once per frame by Canvas3D.
   class ReyMesh {
   public:
      explicit ReyMesh(const Mesh& mesh): _mesh(mesh){} //takes ownership
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
      [[nodiscard]] Box<float> getBoundingBox() const {return Box<float>(GetMeshBoundingBox(_mesh));}
   private:
      Mesh _mesh = {};
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
