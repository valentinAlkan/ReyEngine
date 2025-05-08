#pragma once

#include "Widget.h"
#include "TileEditor.h"

class UI : public ReyEngine::Widget {
public:
   REYENGINE_OBJECT(UI)
   UI();
protected:
   void render2D() const override {}
   void _init() override;
   Widget* _unhandled_input(const ReyEngine::InputEvent&) override;
private:
   std::shared_ptr<TileEditor> _editor;
};