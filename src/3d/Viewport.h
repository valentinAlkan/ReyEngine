#pragma once
#include "BaseWidget.h"
#include "DrawInterface3D.h"
#include "BaseBody.h"

namespace ReyEngine {

   class Viewport
   : public virtual BaseWidget
   , public virtual Internal::Renderer3D
   {
      REYENGINE_OBJECT_BUILD_ONLY(Viewport, BaseWidget)
      , Internal::Renderer3D(name, typeName)
      , _activeCamera(_defaultCamera)
      , _showGrid("showGrid", true)
      {}
   public:
      static std::shared_ptr<Viewport> build(const std::string& name){
         auto me = std::shared_ptr<Viewport>(new Viewport(name));
         return me;
      }
      void _init() override;
      void setUnhandledInputCallback(std::function<Handled(Viewport&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx);
      void setActiveCamera(CameraTransform3D&);
      void deleteActiveCamera();
      inline void addChild3d(std::shared_ptr<BaseBody> child){ReyEngine::Internal::TypeContainer<ReyEngine::Internal::Renderable3D>::addChild(child);}
      inline void addChild2d(std::shared_ptr<BaseWidget> child){ReyEngine::Internal::TypeContainer<ReyEngine::BaseWidget>::addChild(child);}
      inline std::optional<Collisions::RayHit3D> castRayClosest(const Pos<double>& pos){
          if (!_visible) return {};
          return testRayClosest(_activeCamera.get().getRay(pos));
      }
      inline std::vector<Collisions::RayHit3D> castRayAll(const Pos<double>& pos){
          if (!_visible) return {};
          return testRayAll(_activeCamera.get().getRay(pos));}
   protected:
      void addChild();
      void renderChain() override;
      void render2D() const override;
      void renderer3DBegin() override;
      void renderer3DEnd() override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}

      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;

      CameraTransform3D _defaultCamera; //only exists to have something to point to in the event there is no camera defined in the scene tree
      std::reference_wrapper<CameraTransform3D> _activeCamera; //the currently active camera
      std::function<Handled(Viewport&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;


   private:
      BoolProperty _showGrid;

      friend class Window;
   };
}