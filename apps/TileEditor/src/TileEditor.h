#pragma once
#include "TileMap.h"


   class TileEditor : public ReyEngine::TileMap {
   public:
      REYENGINE_OBJECT(GameWorld)
      TileEditor();
   protected:
      ReyEngine::Handled _on_hovered(const TileCoord&) override;
   private:
      std::map<std::string, std::unique_ptr<SpriteAtlas>> spriteAtlases;
   };