#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Canvas, BaseWidget)
      , _renderTarget()
      , _activeCamera(_defaultCamera)
      {}
   public:
      static std::shared_ptr<Canvas> build(const std::string &name) noexcept {
         auto me = std::shared_ptr<Canvas>(new Canvas(name));
         return me;
      }
      //modality
      void setModal(std::shared_ptr<BaseWidget>&);
      void clearModal();
      inline std::optional<std::weak_ptr<BaseWidget>> getModal() { if (_modal) { return _modal;} return std::nullopt;}
      void _init() override;
      void pushScissor(const Rect<R_FLOAT>&);
      void popScissor();
      inline Pos<R_FLOAT> screenToWorld(const Pos<R_FLOAT>& pos) const {return _activeCamera.get().screenToWorld(pos);}
      inline Pos<R_FLOAT> worldToScreen(const Pos<R_FLOAT>& pos) const {return _activeCamera.get().worldToScreen(pos);}
      void setUnhandledInputCallback(std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
      void setActiveCamera(CameraStack2D&);
      void deleteActiveCamera();
   protected:
      void renderBegin(Pos<R_FLOAT>& textureOffset) override;
      void render() const override;
      void renderEnd() override;
      std::optional<std::shared_ptr<BaseWidget>> askHover(const ReyEngine::Pos<R_FLOAT>& globalPos) override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled _process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&);
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      RenderTarget _renderTarget;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.

      std::stack<Rect<R_FLOAT>> _scissorStack;
      CameraStack2D _defaultCamera; //only exists to have something to point to in the event there is no camera defined in the scene tree
      std::reference_wrapper<CameraStack2D> _activeCamera; //the currently active camera
      std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
      friend class Window;
   };
}
