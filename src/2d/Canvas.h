#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Canvas, BaseWidget)
      , _renderTarget()
//      , _activeCamera(_defaultCamera)
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
      Pos<R_FLOAT> screenToWorld(const Pos<R_FLOAT>& pos) const;
      Pos<R_FLOAT> worldToScreen(const Pos<R_FLOAT>& pos) const;
      void setUnhandledInputCallback(std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
      void setActiveCamera(std::shared_ptr<ReyEngine::Camera2D>&);
   protected:
      void renderBegin(Pos<R_FLOAT>& textureOffset) override;
      void render() const override;
      void renderActiveCamera(Pos<R_FLOAT>& textureOffset);
      void renderEnd() override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled __process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) final;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      RenderTarget _renderTarget;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.

      std::stack<Rect<R_FLOAT>> _scissorStack;
      std::weak_ptr<ReyEngine::Camera2D> _activeCamera;
      std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
      friend class Window;
   };
}
