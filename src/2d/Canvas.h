#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Canvas, BaseWidget)
      , _renderTarget()
      , _activeCamera(_defaultCamera)
      {}
   public:
      //modality
      void setModal(std::shared_ptr<BaseWidget>&);
      void clearModal();
      inline std::optional<std::weak_ptr<BaseWidget>> getModal() { if (_modal) { return _modal;} return std::nullopt;}
      void _init() override;
      void pushScissor(const Rect<int>&);
      void popScissor();
      void setUnhandledInputCallback(std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
      void setActiveCamera(CameraTransform2D&);
      void deleteActiveCamera();
   protected:
      void renderBegin(Pos<double>& textureOffset) override;
      void render() const override;
      void renderEnd() override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      RenderTarget _renderTarget;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.

      std::stack<Rect<int>> _scissorStack;
      CameraTransform2D _defaultCamera; //only exists to have something to point to in the event there is no camera defined in the scene tree
      std::reference_wrapper<CameraTransform2D> _activeCamera; //the currently active camera
      std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
      friend class Window;
   };
}
