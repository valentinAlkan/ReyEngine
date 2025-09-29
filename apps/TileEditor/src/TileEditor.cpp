#include "TileEditor.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
TileEditor::TileEditor()
: TileMap(48,48)
{
   _showGrid = true;
   static constexpr char ATLAS_TERRAIN_FILE[] = "test/tiles.png";
   constexpr Pos<int> srcTileSize = {16,16};
   constexpr Pos<int> offset = {2,1};
   constexpr Size<int> padding = {1,1};

   auto atlas = addAtlas("terrain", FileSystem::File(ATLAS_TERRAIN_FILE), srcTileSize, offset, padding);
   if (!atlas) throw std::runtime_error("Sprite atlas file " + std::string(ATLAS_TERRAIN_FILE) + " not found!");
   auto layer = addLayer(atlas.value());

   //setup the world
   if (layer) {
      layer.value()->setTileByCoords({0,0}, {0,0});
      layer.value()->setTileByCoords({0,1}, {0,1});
      layer.value()->setTileByCoords({0,2}, {0,2});
      layer.value()->setTileByCoords({1,0}, {1,0});
      layer.value()->setTileByCoords({1,1}, {1,1});
      layer.value()->setTileByCoords({1,2}, {1,2});
   } else {
      throw std::runtime_error("something bad, not good!");
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled TileEditor::_on_hovered(const TileCoord& tile) {
   getLayer(0).value()->setTileByCoords(tile, {2,1});
   return true;
}