#pragma once
#include "BaseWidget.h"

namespace ReyEngine{
   class Canvas : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Canvas, BaseWidget){}
   public:
      REYENGINE_DEFAULT_BUILD(Canvas);
      //modality
      void setModal(std::shared_ptr<BaseWidget>&);
      void clearModal();
      inline std::optional<std::weak_ptr<BaseWidget>> getModal() { if (_modal) { return _modal;} return std::nullopt;}
      //focusness
      void setFocus(std::shared_ptr<BaseWidget>&);
      void clearFocus();
      inline std::optional<std::weak_ptr<BaseWidget>> getFocus() { if (_focus) { return _focus;} return std::nullopt;}
      void _init() override;
      void pushScissor(const Rect<R_FLOAT>&);
      void popScissor();
      Pos<R_FLOAT> screenToWorld(const Pos<R_FLOAT>& pos) const;
      Pos<R_FLOAT> worldToScreen(const Pos<R_FLOAT>& pos) const;
      void setUnhandledInputCallback(std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
      void setActiveCamera(std::shared_ptr<ReyEngine::Camera2D>&);
      RenderTarget& getRenderTarget(){return _renderTarget;}
      inline Pos<R_FLOAT> getRenderDst() const {return renderDst;}
      inline void setRenderDst(const Pos<R_FLOAT>& dst){renderDst = dst;}
   protected:
      void renderBegin() override;
      void render() const override{};
      void renderActiveCamera(Pos<R_FLOAT>& textureOffset);
      void renderEnd() override;
      void _on_rect_changed() override;
      Handled __process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) final;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      std::optional<std::weak_ptr<BaseWidget>> _focus; //if a widget wishes to collect all input, it can be focused. Only one allowed at a time.
      std::optional<std::weak_ptr<BaseWidget>> _modal; //Focus AND redraw. Only one.

      std::stack<Rect<R_FLOAT>> _scissorStack;
      std::weak_ptr<ReyEngine::Camera2D> _activeCamera;
      RenderTarget _renderTarget;
      std::function<Handled(Canvas&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
   private:
      Pos<R_FLOAT> renderDst;
      friend class Window;
   };
}
