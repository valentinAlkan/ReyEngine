#pragma once
#include "BaseWidget.h"
#include "DrawInterface3D.h"

namespace ReyEngine {
   class Viewport : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Viewport, BaseWidget)
      , _activeCamera(_defaultCamera)
      , _showGrid("showGrid", true)
      {}
   public:
      void _init() override;
      void setUnhandledInputCallback(std::function<Handled(Viewport&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx);
      void setActiveCamera(CameraTransform3D&);
      void deleteActiveCamera();
      static std::shared_ptr<Viewport> build(const std::string& instanceName);
   protected:
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
      void render() const override;
      void renderEnd() override;
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