#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
   public:
      REYENGINE_OBJECT(Canvas, BaseWidget)
      , _renderTarget()
      {}
   public:
      //modality
      void setModal(std::shared_ptr<BaseWidget>&);
      void clearModal();
      inline std::optional<std::weak_ptr<BaseWidget>> getModal() { if (_modal) { return _modal;} return std::nullopt;}
      void _init() override {_renderTarget.setSize({0, 0});} //todo: make protected
   protected:
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
      void render() const override;
      void renderEnd() override;
      inline void _on_rect_changed() override {_renderTarget.setSize(_rect.value.size());}
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      ReyEngine::RenderTarget _renderTarget;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.
      friend class Window;
   };
}
