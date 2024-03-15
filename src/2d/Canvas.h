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
      void _init() override;
      void pushScissor(const ReyEngine::Rect<int>&);
      void popScissor();
   protected:
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
      void render() const override;
      void renderEnd() override;
      void _on_rect_changed() override;
      void _process(float dt) override {}
      void registerProperties() override{}
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      ReyEngine::RenderTarget _renderTarget;
      std::optional<std::weak_ptr<BaseWidget>> _modal; //if a widget wishes to collect all input, it can be modal. Only one allowed at a time.

//      struct ScissorData {
//         ScissorData(const Rect<int>& newArea): scissorArea(newArea){};
//         ScissorData(const Rect<int>& newArea, const ScissorData& top)
//         : scissorArea(newArea)
//         , overlap(top.overlap.getOverlap(newArea).value_or(Rect<int>(0,0,0,0))){}
//         const ReyEngine::Rect<int> scissorArea;
//         const ReyEngine::Rect<int> overlap;
//      };

      std::stack<Rect<int>> _scissorStack;
      friend class Window;
   };
}
