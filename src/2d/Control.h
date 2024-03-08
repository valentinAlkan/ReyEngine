#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Control : public BaseWidget {

public:
   REYENGINE_OBJECT(Control, BaseWidget){}
public:
    void render() const override;
    void _process(float dt) override;
    Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
    void _on_mouse_enter() override;
    void _on_mouse_exit() override;
    void _on_rect_changed() override;
    void registerProperties() override {};
    void setRenderCallback(std::function<const void(const Control&)> fx){renderCallback = fx;}
    void setProcessCallback(std::function<void(const Control&, float)> fx){processCallback = fx;setProcess(true);}
    void setUnhandledInputCallback(std::function<Handled(const Control&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
    void setMouseEnterCallback(std::function<void(const Control&)> fx){mouseEnterCallback=fx; acceptsHover=true;}
    void setMouseExitCallback(std::function<void(const Control&)> fx){mouseExitCallback=fx; acceptsHover=true;}
    void setRectChangedCallback(std::function<void(const Control&)> fx){rectChangedCallback=fx;}
protected:
    std::function<void(const Control&)> mouseEnterCallback;
    std::function<void(const Control&)> mouseExitCallback;
    std::function<void(const Control&)> renderCallback;
    std::function<void(const Control&, float dt)> processCallback;
    std::function<void(const Control&)> rectChangedCallback;
    std::function<Handled(const Control&, const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
};