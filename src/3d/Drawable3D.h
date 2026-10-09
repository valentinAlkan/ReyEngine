#pragma once
#include <optional>
#include <vector>
#include "Positionable3D.h"
#include "Resources3D.h"

namespace ReyEngine {
   class Canvas3D;

   /// 3D counterpart of RenderContext. Tracks the global transform of whatever is currently being drawn
   /// as Canvas3D walks the tree, so nodes can draw in their own local space.
   class RenderContext3D {
   public:
      /// defaultShader is used for any material that hasn't been given a shader of its own (Canvas3D passes its
      /// lighting shader when there are lights). Null means raylib's default, unlit, shader.
      explicit RenderContext3D(const std::shared_ptr<ReyShader>& defaultShader = nullptr): _defaultShader(defaultShader){}
      void push(const Transform3D& local){_stack.push_back(_stack.empty() ? local : local * _stack.back());}
      //doesn't check for empty!
      void pop(){_stack.pop_back();}
      [[nodiscard]] Transform3D getTransform() const {return _stack.empty() ? Transform3D() : _stack.back();}
      /// draw a mesh in the current local space
      void drawMesh(const ReyMesh& mesh, const ReyMaterial& material) const {
         DrawMesh(mesh.getMesh(), resolve(material.getMaterial(), material.getShader() != nullptr), getTransform().matrix);
      }
      /// draw a model in the current local space, tinting all its materials
      void drawModel(const ReyModel& reyModel, const ColorRGBA& tint = Colors::white) const {
         const auto& model = reyModel.getModel();
         auto xform = MatrixMultiply(model.transform, getTransform().matrix);
         for (int i = 0; i < model.meshCount; i++) {
            auto material = model.materials[model.meshMaterial[i]];
            Color& color = material.maps[MATERIAL_MAP_DIFFUSE].color;
            Color original = color;
            color = ColorTint(color, (Color)tint);
            DrawMesh(model.meshes[i], resolve(material, false), xform);
            color = original;
         }
      }
   private:
      [[nodiscard]] Material resolve(Material material, bool hasOwnShader) const {
         if (!hasOwnShader && _defaultShader) material.shader = _defaultShader->getShader();
         return material;
      }
      std::shared_ptr<ReyShader> _defaultShader;
      std::vector<Transform3D> _stack;
   };

   namespace Internal {
      /// 3D counterpart of Drawable2D: something in the tree that has a transform and can be drawn by a Canvas3D.
      class Drawable3D
         : public ReyObject
         , public Positionable3D {
      public:
         REYENGINE_OBJECT(Drawable3D)
         [[nodiscard]] inline std::optional<Canvas3D*> getCanvas3D(){if (_canvas3D) { return _canvas3D; } else {return std::nullopt;}}
         [[nodiscard]] inline std::optional<const Canvas3D*> getCanvas3D() const {return const_cast<Drawable3D*>(this)->getCanvas3D();}
         [[nodiscard]] inline bool getVisible() const {return _visible;}
         inline void setVisible(bool newValue){ _visible = newValue; __on_visibility_changed();}
      protected:
         virtual void render3D(RenderContext3D&) const = 0;
         virtual void render3DBegin(RenderContext3D&){}
         virtual void render3DEnd(RenderContext3D&){}
         virtual void __on_visibility_changed(){}
         bool _visible = true;
      private:
         Canvas3D* _canvas3D = nullptr;
      friend class ReyEngine::Canvas3D;
      };
   }
}
