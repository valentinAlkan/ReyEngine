#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Control : public BaseWidget {

public:
   REYENGINE_OBJECT(Control, BaseWidget){}
public:
    void render() const override;
    void _process(float dt) override;
    Handled _unhandled_input(InputEvent& event) override;
    void _on_mouse_enter() override;
    void _on_mouse_exit() override;
    void registerProperties() override {};
    void setRenderCallback(std::function<const void()> fx){renderCallback = fx;}
    void setProcessCallback(std::function<void()> fx){processCallback = fx;setProcess(true);}
    void setUnhandledInputCallback(std::function<Handled(const InputEvent&)> fx){unhandledInputCallback = fx;}
    void setMouseEnterCallback(std::function<void()> fx){mouseEnterCallback=fx; acceptsHover=true;}
    void setMouseExitCallback(std::function<void()> fx){mouseExitCallback=fx; acceptsHover=true;}
protected:
    std::function<void()> mouseEnterCallback;
    std::function<void()> mouseExitCallback;
    std::function<void()> renderCallback;
    std::function<void()> processCallback;
    std::function<Handled(const InputEvent&)> unhandledInputCallback;
};