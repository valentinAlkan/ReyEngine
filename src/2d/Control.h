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
    void setRenderCallback(std::function<const void()> fx){renderCallback = fx;}
    void setProcessCallback(std::function<void()> fx){processCallback = fx;setProcess(true);}
    void setUnhandledInputCallback(std::function<Handled(const InputEvent&, const std::optional<UnhandledMouseInput>&)> fx){unhandledInputCallback = fx;}
    void setMouseEnterCallback(std::function<void()> fx){mouseEnterCallback=fx; acceptsHover=true;}
    void setMouseExitCallback(std::function<void()> fx){mouseExitCallback=fx; acceptsHover=true;}
    void setRectChangedCallback(std::function<void()> fx){rectChangedCallback=fx;}
protected:
    std::function<void()> mouseEnterCallback;
    std::function<void()> mouseExitCallback;
    std::function<void()> renderCallback;
    std::function<void()> processCallback;
    std::function<void()> rectChangedCallback;
    std::function<Handled(const InputEvent&, const std::optional<UnhandledMouseInput>&)> unhandledInputCallback;
};