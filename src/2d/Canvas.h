#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
      REYENGINE_OBJECT_CUSTOM_BUILD(Canvas, BaseWidget, std::tuple<const std::string&, RenderTarget&>);
   public:
      REYENGINE_DEFAULT_BUILD(Canvas);
      static std::shared_ptr<Canvas> build(const std::string &name, RenderTarget& renderTarget) noexcept {
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
      Canvas(const std::string& instanceName)
      : REYENGINE_CTOR_INIT_LIST(instanceName, BaseWidget)
      , _renderTargetPtr(std::make_unique<RenderTarget>())
      , _renderTarget(*_renderTargetPtr)
      {
         std::cout << &(*_renderTargetPtr) << std::endl;
      }
      Canvas(const std::string& instanceName, RenderTarget& renderTarget): Canvas(instanceName){
         _renderTarget = renderTarget;
      }
      void renderBegin(Pos<R_FLOAT>& textureOffset) override;
      void render() const override;
      void renderActiveCamera(Pos<R_FLOAT>& textureOffset);
      void renderEnd() override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled __process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) final;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.

      std::stack<Rect<R_FLOAT>> _scissorStack;
      std::weak_ptr<ReyEngine::Camera2D> _activeCamera;
      std::unique_ptr<RenderTarget> _renderTargetPtr; // if not using an external render target, this will be initialized. Otherwise it is null. (alo: declare this first so its initialized before _renderTarget)
      RenderTarget& _renderTarget;
      std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
   private:
      friend class Window;
   };
}
