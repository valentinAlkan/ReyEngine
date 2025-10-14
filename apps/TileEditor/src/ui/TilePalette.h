#pragma once
#include "Panel.h"
#include "TileMap.h"

//class TilePaletteRendered : public ReyEngine::TileMap {
//public:
//   REYENGINE_OBJECT(TilePalette)
//   TilePaletteRendered();
//   void render2D() const override;
//   Widget* _unhandled_input(const ReyEngine::InputEvent&) override;
//private:
//   void _init() override;
//   ReyEngine::Vec2<int> _selectedTile;
//   ReyEngine::Vec2<int> _hoveredTile;
//   static constexpr ReyEngine::Size<float> THUMBNAIL_SIZE = {32, 32};
//};


class TilePalette : public ReyEngine::Widget {
public:
   REYENGINE_OBJECT(TilePalette)
   TilePalette();
   void render2D() const override;
   Widget* _unhandled_input(const ReyEngine::InputEvent&) override;
   void _on_rect_changed() override;
private:
   void _init() override;
   ReyEngine::Vec2<int> _selectedTile;
   ReyEngine::Vec2<int> _hoveredTile;
   static constexpr ReyEngine::Size<float> THUMBNAIL_SIZE = {32, 32};
};