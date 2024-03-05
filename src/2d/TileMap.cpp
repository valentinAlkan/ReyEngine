#include "TileMap.h"
#include "Application.h"
//#include <pair>

using namespace ReyEngine;
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_init() {
   if (!_renderTarget.ready()) {
      _renderTarget.setSize(getSize());
   }
   _ready = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_on_rect_changed() {
   if (_ready) {
      _renderTarget.setSize(getSize());
   }
}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void TileMap::renderBegin(ReyEngine::Pos<double>& renderOffset) {
////   Application::instance().getWindow()->pushRenderTarget(_renderTarget);
////   ClearBackground(ReyEngine::Colors::none);
////   renderOffset += _rect.value.pos();
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void TileMap::renderEnd() {
//   Application::instance().getWindow()->popRenderTarget();
//   _drawRenderTargetRect(_renderTarget, Rect<int>(_renderTarget.getSize()), {0,0});
//}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render() const {
   //draw grid
//   _drawRectangle(_rect.value.toSizeRect(), Colors::blue);
   if (_showGrid){
      switch (_gridType.value){
         case GridType::SQUARE:{
            auto xLineCount = getWidth() / _gridWidth + 1;
            auto yLineCount = getHeight() / _gridHeight + 1;
            for (int x=0; x<xLineCount; x++) {
               auto _x = x*_gridWidth;
               auto linex = Line<int>({_x, 0}, {_x, getHeight()});
               _drawLine(linex, 1, theme->background.colorSecondary);
               for (int y = 0; y < yLineCount; y++){
                  auto _y = y*_gridHeight;
                  auto liney = Line<int>({0, _y}, {getWidth(), _y});
                  _drawLine(liney, 1, theme->background.colorSecondary);
               }
            }
            break;}
         case GridType::HEX:
            break;
         case GridType::SQUARE_OFFSET:
            break;
      }
   }

   //draw all tiles in the layer
   for (auto& layer : _layers){
      for (auto& [x, yMap] : layer.second.tiles){
         for (auto& [y, index] : yMap){
            auto pos = getPos({x,y});
            _drawText("a", pos, theme->font);
            auto& tex = layer.second.atlas.texture;
            _drawTextureRect(tex, {_gridWidth * (int)index, _gridHeight, _gridWidth, _gridHeight}, pos);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<TileMap::LayerIndex> TileMap::addLayer(const FileSystem::File& file) {
   //try to add the texture
   if (!file.exists()) {
      Application::printError() << "TileMap::addTexture - file " + file.abs() + " does not exist!" << endl;
      return nullopt;
   }
   //load the texture
   TileMapLayer newLayer(file);
   //store it
   auto newIndex = getNextLayerIndex();
   _layers.insert(std::pair<LayerIndex, TileMapLayer>(getNextLayerIndex(), std::move(newLayer)));
   return newIndex;
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
TileMap::TileMapLayer& TileMap::getLayer(ReyEngine::TileMap::LayerIndex idx) {
   auto found = _layers.find(idx);
   if (found == _layers.end()){
      throw runtime_error("TileMap layer index " + to_string(idx) + " not found!");
   }
   return found->second;
}

/////////////////////////////////////////////////////////////////////////////////////////
TileMap::TileCoord TileMap::getCoord(Pos<int> localPos) const {
   auto x = localPos.x / _gridWidth.value;
   auto y = localPos.y / _gridHeight.value;
   return {x,y};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<int> TileMap::getPos(TileCoord coord) const {
   auto x = coord.x * _gridWidth.value;
   auto y = coord.y * _gridHeight.value;
   return {x,y};
}

////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<TileMap::TileIndex> TileMap::TileMapLayer::getTileIndex(const TileCoord &pos) {
   try {
      return tiles.at(pos.x).at(pos.y);
   } catch (const out_of_range& e) {
      Application::printDebug() << "Tile layer " << atlas.filePath.str() << " has no index at coordinates " << pos << endl;
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::TileMapLayer::setTileIndex(const TileCoord& coords, TileIndex index) {
   tiles[coords.x][coords.y] = index;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::registerProperties() {
   registerProperty(_showGrid);
   registerProperty(_gridType);
   registerProperty(_gridHeight);
   registerProperty(_gridWidth);
}