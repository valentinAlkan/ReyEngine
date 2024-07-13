#pragma once
#include "BaseWidget.h"
#include "Application.h"

namespace ReyEngine{
   class Control : public BaseWidget {

   public:
   public:
      static constexpr char TYPE_NAME[] = "Control";
      std::string _get_static_constexpr_typename()
      override{
         return
               TYPE_NAME;
      }public:
      static std::shared_ptr<Component> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
         auto retval = std::shared_ptr<Control>(new Control(instanceName));
         retval->Component::_deserialize(properties);
         retval->Component::_on_deserialize(properties);
         return retval;
      }
      Control(const std::string &name) : Control(name, _get_static_constexpr_typename()) {}
   protected:
      void _register_parent_properties()
      override{
         BaseWidget::_register_parent_properties();
         BaseWidget::registerProperties();
      }
      Control(const std::string &name, const std::string &typeName) : BaseWidget(name, typeName), NamedInstance(name, typeName){}
   public:
       void _process(float dt) override;
       Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
       void _on_mouse_enter() override;
       void _on_mouse_exit() override;
       void _on_rect_changed() override;
       void registerProperties() override {};
       void setRenderCallback(std::function<void(const Control&)> fx){renderCallback = fx;}
       void setProcessCallback(std::function<void(Control&, float)> fx){processCallback = fx;setProcess(true);}
       void setUnhandledInputCallback(std::function<Handled(Control&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
       void setMouseEnterCallback(std::function<void(Control&)> fx){mouseEnterCallback=fx; acceptsHover=true;}
       void setMouseExitCallback(std::function<void(Control&)> fx){mouseExitCallback=fx; acceptsHover=true;}
       void setRectChangedCallback(std::function<void(Control&)> fx){rectChangedCallback=fx;}
       void setScissorArea(const ReyEngine::Rect<int>&);
       void clearScissorArea();
   protected:
       bool _doScissor = false;
       void render() const override;
       void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
       void renderEnd() override;
       std::function<void(Control&)> mouseEnterCallback;
       std::function<void(Control&)> mouseExitCallback;
       std::function<void(const Control&)> renderCallback;
       std::function<void(Control&, float dt)> processCallback;
       std::function<void(Control&)> rectChangedCallback;
       std::function<Handled(Control&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
       ReyEngine::Rect<int> scissorArea;
   };
}