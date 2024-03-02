#include "TileMap.h"
#include "Application.h"
//#include <pair>

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render() const {
   //draw grid
   if (_showGrid){
      switch (_gridType.value){
         case GridType::SQUARE:
            static constexpr int LINE_COUNT = 20;
            static constexpr int CELL_SIZE = 64;
            for (int x=0; x<LINE_COUNT; x++) {
               auto _x = x*CELL_SIZE;
               auto linex = Line<int>({_x, 0}, {_x, getScreenSize().y});
               _drawLine(linex, 1, theme->background.colorPrimary);
               for (int y = 0; y < LINE_COUNT; y++){
                  auto _y = y*CELL_SIZE;
                  auto liney = Line<int>({0, _y}, {getScreenSize().x, _y});
                  _drawLine(liney, 1, theme->background.colorPrimary);
               }
            }
            break;
         case GridType::HEX:
            break;
         case GridType::SQUARE_OFFSET:
            break;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<TileMap::LayerIndex> TileMap::addTexture(const FileSystem::File& file) {
   //try to add the texture
   if (!file.exists()) {
      Application::printError() << "TileMap::addTexture - file " + file.abs() + " does not exist!" << endl;
      return nullopt;
   }
   //load the texture
   pair<LayerIndex , TileMapLayer> pair = std::make_pair(getNextLayerIndex(), file);
   _layers[getNextLayerIndex()] = TileMapLayer(file);
   newLayer.atlas.

}

/////////////////////////////////////////////////////////////////////////////////////////
TileMap::LayerIndex TileMap::getNextLayerIndex() {
   //find the first available tilemap index
   LayerIndex available = 0;
   for (const auto& [k,v] : _layers){
      if (k != available){
         return available;
      }
      available++;
   }
   return available;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::registerProperties() {
   registerProperty(_showGrid);
   registerProperty(_gridType);
}