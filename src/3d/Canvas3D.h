#pragma once
#include <vector>
#include "Canvas.h"
#include "Light3D.h"

namespace ReyEngine {

   class Canvas3D : public Canvas {
   public:
      static constexpr size_t MAX_LIGHTS = 8;
      REYENGINE_OBJECT(Canvas3D)
      Canvas3D()
      : _camera3D({{0,0,5}, {0,0,0}, {0,1,0}, 45.0f, CAMERA_PERSPECTIVE})
      {}
      // The camera the 3D scene is viewed through. Like Canvas::getCamera (which still applies to any 2D widgets
      // in the background layer), this is the raylib camera itself.
      Camera3D& getCamera3D(){return _camera3D;}
      const Camera3D& getCamera3D() const {return _camera3D;}
      // 3D counterparts of toBackgroundPos/toForegroundPos.
      // The ray from the camera through a point in canvas space, in scene space
      [[nodiscard]] Ray3D toBackgroundRay(const Pos<float>& canvasPos) const;
      // Where a point in scene space appears on the canvas
      [[nodiscard]] Pos<float> toForegroundPos(const Pos3<float>& scenePos) const;
      using Canvas::toForegroundPos;

      // the nearest pickable, visible node under a point on the canvas (or along a scene space ray)
      [[nodiscard]] std::optional<Hit3D> pick(const Pos<float>& canvasPos) const {return pick(toBackgroundRay(canvasPos));}
      [[nodiscard]] std::optional<Hit3D> pick(const Ray3D& sceneRay) const;

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
      // the 3D scene is the background layer for input too: 2D widgets in it get first refusal, then the 3D nodes,
      // which are offered input just as widgets are (see Node3D).
      Handled _processBackgroundInput(const InputEvent&) override;
      // nothing in the scene can be under the mouse once it has left the canvas
      void _on_mouse_exit() override {setHover(nullptr);}

   private:
      void loadGpuResources();
      void uploadLightUniforms(const std::vector<const Light3D*>& lights);

      Camera3D _camera3D;
      Node3D* _mousePick = nullptr; //while input is being dispatched, the node under the mouse
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
      friend class Node3D;
   };
}
