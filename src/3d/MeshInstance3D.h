#pragma once
#include "Node3D.h"

namespace ReyEngine {
   /// Draws a mesh with a material. The 3D counterpart of TextureRect/Sprite.
   /// Meshes and materials are shared, so many instances can draw the same mesh at different transforms.
   class MeshInstance3D : public Node3D {
   public:
      REYENGINE_OBJECT(MeshInstance3D)
      MeshInstance3D() = default;
      MeshInstance3D(const std::shared_ptr<ReyMesh>& mesh, const std::shared_ptr<ReyMaterial>& material = nullptr)
      : _mesh(mesh)
      , _material(material)
      {}
      //convenience: give this instance its own material of the given color
      MeshInstance3D(const std::shared_ptr<ReyMesh>& mesh, const ColorRGBA& color)
      : MeshInstance3D(mesh, std::make_shared<ReyMaterial>(color))
      {}
      void setMesh(const std::shared_ptr<ReyMesh>& mesh){_mesh = mesh;}
      [[nodiscard]] const std::shared_ptr<ReyMesh>& getMesh() const {return _mesh;}
      void setMaterial(const std::shared_ptr<ReyMaterial>& material){_material = material;}
      [[nodiscard]] const std::shared_ptr<ReyMaterial>& getMaterial() const {return _material;}
      /// bounds of the mesh in local space
      [[nodiscard]] Box<float> getBoundingBox() const {return _mesh ? _mesh->getBoundingBox() : Box<float>();}
   protected:
      void render3D(RenderContext3D&) const override;
      std::shared_ptr<ReyMesh> _mesh;
      std::shared_ptr<ReyMaterial> _material;
   };

   /// Draws a model (a set of meshes and their materials, typically loaded from a file).
   class ModelInstance3D : public Node3D {
   public:
      REYENGINE_OBJECT(ModelInstance3D)
      ModelInstance3D() = default;
      explicit ModelInstance3D(const std::shared_ptr<ReyModel>& model): _model(model){}
      explicit ModelInstance3D(const FileSystem::File& file): _model(std::make_shared<ReyModel>(file)){}
      void setModel(const std::shared_ptr<ReyModel>& model){_model = model;}
      [[nodiscard]] const std::shared_ptr<ReyModel>& getModel() const {return _model;}
      void setTint(const ColorRGBA& tint){_tint = tint;}
      [[nodiscard]] const ColorRGBA& getTint() const {return _tint;}
      /// bounds of the model in local space
      [[nodiscard]] Box<float> getBoundingBox() const {return _model ? _model->getBoundingBox() : Box<float>();}
   protected:
      void render3D(RenderContext3D&) const override;
      std::shared_ptr<ReyModel> _model;
      ColorRGBA _tint = Colors::white;
   };
}
