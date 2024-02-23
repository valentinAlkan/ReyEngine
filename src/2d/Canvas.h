#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
   public:
      REYENGINE_OBJECT(Canvas, BaseWidget)
      , target()
      {}
   public:
      //modality
      inline void setModal(std::shared_ptr<BaseWidget>& newModal){_modal = newModal;}
      inline void clearModal(){_modal.reset();}
      inline std::optional<std::weak_ptr<BaseWidget>> getModal() { if (_modal) { return _modal;} return std::nullopt;}
   protected:
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
      void render() const override {}
      void renderEnd() override;
      inline void _on_rect_changed() override {target.setSize(_rect.value.size());}
      void _process(float dt) override {}
      void registerProperties() override{
         //register properties specific to your type here.
      }
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      ReyEngine::RenderTarget target;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.
   };
}
