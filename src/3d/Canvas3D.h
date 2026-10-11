#pragma once
#include <vector>
#include "Canvas.h"
#include "Light3D.h"

namespace ReyEngine {

   class Canvas3D : public Canvas {
   public:
      static constexpr size_t MAX_LIGHTS = 8;
      // A 2D Canvas that is the child of a 3D node is drawn as a flat panel in that node's local XY plane, facing +Z,
      // and is offered input wherever the mouse falls on it. Within the plane, the canvas is placed by its own 2D
      // transform, in pixels, with PIXELS_PER_UNIT pixels to a unit and y pointing down as in 2D (so by default its top
      // left corner is at the node's origin). Scale or move the node to size or place the panel.
      static constexpr R_FLOAT PIXELS_PER_UNIT = 100;
      // where a point on such a panel (in the canvas's space) lies in the space of its node, and vice versa
      [[nodiscard]] static Pos3<R_FLOAT> panelToNodePos(const Canvas* panel, const Pos<R_FLOAT>& panelPos);
      [[nodiscard]] static Pos<R_FLOAT> nodeToPanelPos(const Canvas* panel, const Pos3<R_FLOAT>& nodePos); //ignores z
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
      // draw the 2D canvases in a 3D subtree into their render targets, ready to be drawn as panels in the scene
      static void renderPanels(RenderContext&, Internal::Drawable3D*);
      // draw a panel in the current local space of the 3D context
      static void drawPanel(const RenderContext3D&, Canvas* panel);
      // the distance along a ray (in the space of a panel's node) to where it crosses the panel's plane, if it does
      [[nodiscard]] static std::optional<R_FLOAT> intersectPanelPlane(const Ray3D& nodeRay);
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
