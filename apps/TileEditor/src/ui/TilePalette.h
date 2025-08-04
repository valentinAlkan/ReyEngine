#pragma once

#include "Layout.h"

class TilePalette : public ReyEngine::Layout {
public:
   REYENGINE_OBJECT(TilePalette)
   TilePalette();
   void render2D() const override;
   Widget* _unhandled_input(const ReyEngine::InputEvent&) override;

private:
//   std::shared_ptr<
   ReyEngine::Vec2<int> _selectedTile;
   ReyEngine::Vec2<int> _hoveredTile;
   static constexpr ReyEngine::Size<float> THUMBNAIL_SIZE = {32, 32};
};