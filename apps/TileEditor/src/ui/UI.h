#pragma once

#include "TileEditor.h"
#include "TilePalette.h"

class UI : public ReyEngine::Widget {
public:
   REYENGINE_OBJECT(UI)
   UI(std::shared_ptr<TileEditor>& editor);
protected:
   void render2D() const override {}
   void _init() override;
   Widget* _unhandled_input(const ReyEngine::InputEvent&) override;
   void _on_rect_changed() override;
private:
   std::shared_ptr<TileEditor> _editor;
   std::shared_ptr<TilePalette> _palette;
};