#pragma once
#include <vector>
#include "Canvas.h"
#include "Light3D.h"

namespace ReyEngine {

   // Simple 3D camera wrapper (named to avoid clashing with raylib's global Camera3D)
   struct ReyCamera3D {
      Vec3<float> position{0,0,5};
      Vec3<float> target{0,0,0};
      Vec3<float> up{0,1,0};
      float fovy = 45.0f; // degrees
   };

   class Canvas3D : public Canvas {
   public:
      static constexpr size_t MAX_LIGHTS = 8;
      REYENGINE_OBJECT(Canvas3D)
      Canvas3D() = default;
      // expose camera
      ReyCamera3D& getCamera3D(){return _camera3D;}
      const ReyCamera3D& getCamera3D() const {return _camera3D;}

      // lighting comes from Light3D nodes in the scene (at most MAX_LIGHTS are used). With none, everything is drawn unlit.
      void setAmbientLight(const ColorRGBA& color){_ambient = color;}
      const ColorRGBA& getAmbientLight() const {return _ambient;}

   protected:
      // the 3D scene is the background layer. 2D widgets (in either layer) are drawn over it, like a HUD.
      void renderBackgroundLayer(RenderContext&) override;
      void __on_descendant_added_to_tree(TypeNode* child) override;
      void __on_descendant_removed_from_tree(TypeNode* child) override;
      // draw a 3D node and its 3D descendants. The 3D counterpart of Canvas::doRender
      static void doRender3D(RenderContext3D&, Internal::Drawable3D*);
      // find the visible lights in a 3D subtree
      static void collectLights(Internal::Drawable3D*, std::vector<const Light3D*>&);

   private:
      void loadGpuResources();
      void uploadLightUniforms(const std::vector<const Light3D*>& lights);

      ReyCamera3D _camera3D;
      ColorRGBA _ambient = {40, 40, 40, 255};

      // GPU resources, created lazily on first render since they need a GL context
      bool _gpuLoaded = false;
      std::shared_ptr<ReyShader> _lightShader;
      template <typename T> using Uniform = ReyShader::ShaderValue<ReyShader::Uniform, T>;
      struct {
         Uniform<Vec3<float>> viewPos, ambient, lightVector, lightColor;
         Uniform<int> lightCount, lightType;
         Uniform<float> lightRange;
      } _uniforms;
   };
}
