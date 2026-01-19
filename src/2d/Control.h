#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Control : public Widget {
   public:
       REYENGINE_OBJECT(Control);
       void _process(float dt) override;
       Widget* _unhandled_input(const InputEvent&) override;
       void _on_mouse_enter() override;
       void _on_mouse_exit() override;
//       void _on_rect_changed() override;
//       void registerProperties() override {};
       void setRenderCallback(std::function<void(const Control&)> fx){renderCallback = fx;}
       void setProcessCallback(std::function<void(Control&, float)> fx, bool enableProcess=true){
          processCallback = fx;
          if (enableProcess){
             setProcess(true);
          }
       }
       void setUnhandledInputCallback(std::function<Widget*(Control&, const InputEvent&)> fx){unhandledInputCallback = fx;}
       void setMouseEnterCallback(std::function<void(Control&)> fx){mouseEnterCallback=fx; _acceptsHover=true;}
       void setMouseExitCallback(std::function<void(Control&)> fx){mouseExitCallback=fx; _acceptsHover=true;}
       void setRectChangedCallback(std::function<void(Control&)> fx){rectChangedCallback=fx;}
       void setScissorArea(const ReyEngine::Rect<int>&);
       void clearScissorArea();
   protected:
       void _on_rect_changed() override;
       bool _doScissor = false;
       void render2D() const override;
       void render2DBegin() override;
       void render2DEnd() override;
       std::function<void(Control&)> mouseEnterCallback;
       std::function<void(Control&)> mouseExitCallback;
       std::function<void(const Control&)> renderCallback;
       std::function<void(Control&, float dt)> processCallback;
       std::function<void(Control&)> rectChangedCallback;
       std::function<Widget*(Control&, const InputEvent&)> unhandledInputCallback;
       ReyEngine::Rect<int> scissorArea;
   };
}