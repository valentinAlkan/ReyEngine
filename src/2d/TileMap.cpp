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

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render() const {
   //draw all tiles in the layer
   for (auto& layer : _layers){
      for (auto& [x, yMap] : layer.second.tiles){
         for (auto& [y, index] : yMap){
            auto pos = getCellPos({x,y});
            const auto& srcSize = layer.second.atlas.tileSize;
            auto srcRectOpt = layer.second.atlas.getTile(index);
            if (srcRectOpt) {
               auto& srcRect = srcRectOpt.value();
               auto dstRect = Rect<int>(pos, {_gridWidth, _gridHeight});
               auto &tex = layer.second.atlas.texture;
               _drawTextureRect(tex, srcRect, dstRect, 0.0, Colors::none);
            }
         }
      }
   }

   //draw grid on top of tiles
   if (_showGrid){
      _drawRectangleLines(_rect.value.toSizeRect(), 1.0, theme->background.colorSecondary);
      switch (_gridType.value){
         case GridType::SQUARE:{
            auto xLineCount = getWidth() / _gridWidth;
            auto yLineCount = getHeight() / _gridHeight;
            for (int x=1; x<xLineCount; x++) {
               auto _x = x*_gridWidth;
               auto linex = Line<int>({_x, 0}, {_x, getHeight()});
               _drawLine(linex, 1, theme->background.colorSecondary);
               for (int y = 1; y < yLineCount +1; y++){
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
TileMap::TileCoord TileMap::getCell(Pos<int> localPos) const {
   auto x = localPos.x / _gridWidth.value;
   auto y = localPos.y / _gridHeight.value;
   return {x,y};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<int> TileMap::getCellPos(TileCoord coord) const {
   auto x = coord.x * _gridWidth.value;
   auto y = coord.y * _gridHeight.value;
   return {x,y};
}

////////////////////////////////////////////////////////////////////////////////////////
void TileMap::setGridSize(Size<int> size) {
   _gridWidth = size.x;
   _gridHeight = size.y;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled TileMap::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   if (mouse && mouse->isInside) {
      switch (event.eventId) {
         case InputEventMouseMotion::getUniqueEventId(): {
            auto mmEvent = event.toEventType<InputEventMouseMotion>();
            auto cellPos = getCell(mmEvent.globalPos);
            if (cellPos != currentHover) {
               currentHover = cellPos;
               EventTileMapCellHovered event(toEventPublisher(), currentHover, {getCellPos(currentHover), {_gridWidth, _gridHeight}});
               publish(event);
               return true;
            }
         break;
         }
         case InputEventMouseButton::getUniqueEventId():
            auto mbEvent = event.toEventType<InputEventMouseButton>();
            if (mbEvent.isDown && mbEvent.button == InputInterface::MouseButton::LEFT){
               EventTileMapCellClicked event(toEventPublisher(), currentHover, {getCellPos(currentHover), {_gridWidth, _gridHeight}});
               publish(event);
               return true;
            }
         break;
      }
   }
   return false;
}

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
void TileMap::TileMapLayer::removeTileIndex(const TileCoord &pos) {
   auto xFound = tiles.find(pos.x);
   if (xFound == tiles.end()) return;
   auto& ymap = xFound->second;
   auto yFound = ymap.find(pos.y);
   if (yFound == ymap.end()) return;
   ymap.erase(yFound);
   //if the ymap is now empty, delete it
   if (xFound->second.empty()){
      tiles.erase(xFound);
   }
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