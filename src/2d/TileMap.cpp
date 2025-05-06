#include "TileMap.h"
#include "Application.h"

using namespace ReyEngine;
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render2D() const {
//   if (!_needsRedraw){ return;}
   auto& mutableThis = const_cast<TileMap&>(*this);
   //draw all tiles in the layer
   for (auto& layer : _layers){
      for (auto& [x, yMap] : layer.second->tiles){
         for (auto& [y, index] : yMap){
            auto pos = getCellPos({x,y});
            auto srcRect = layer.second->atlas->getTile(index);
            auto dstRect = Rect<R_FLOAT>(pos, {(R_FLOAT)_tileSize.x, (R_FLOAT)_tileSize.y});
            auto &tex = layer.second->atlas->texture;
            drawTexture(tex, srcRect, dstRect, Colors::none);
         }
      }
   }

   //draw grid on top of tiles
   if (_showGrid){
      drawRectangleLines(getRect().toSizeRect(), 1.0, theme->background.colorSecondary);
      switch (_gridType){
         case GridType::SQUARE:{
            auto xLineCount = getWidth() / (float)_tileSize.x;
            auto yLineCount = getHeight() / (float)_tileSize.y;
            for (int x=1; x<(int)xLineCount; x++) {
               auto _x = x*_tileSize.x;
               auto linex = Line<R_FLOAT>({(float)_x, 0}, {(float)_x, getHeight()});
               drawLine(linex, 1, theme->background.colorSecondary);
               for (int y = 1; y < yLineCount +1; y++){
                  auto _y = y*_tileSize.y;
                  auto liney = Line<R_FLOAT>({0, (float)_y}, {getWidth(), (float)_y});
                  drawLine(liney, 1, theme->background.colorSecondary);
               }
            }
            break;}
         case GridType::HEX:
            break;
         case GridType::SQUARE_OFFSET:
            break;
      }
   }
//   mutableThis._renderTarget.endRenderMode();
   mutableThis._needsRedraw = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
optional<TileMap::SpriteAtlas*> TileMap::addAtlas(const std::string& atlasName, const FileSystem::File& file,
                                                  const Size<int>& tileSize, const Pos<int>& offset, const Size<int>& padding) {
   //try to add the texture
   if (!file.exists()) {
      Logger::error() << "TileMap::addAtlas - file " + file.abs() + " does not exist!" << endl;
      return nullopt;
   }
   auto retval = _atlases.try_emplace(atlasName, unique_ptr<SpriteAtlas>(new SpriteAtlas(file, tileSize, offset, padding)));
   return retval.first->second.get();
}

/////////////////////////////////////////////////////////////////////////////////////////
optional<TileMap::SpriteAtlas *> TileMap::getAtlas(const string& atlasName) {
   auto found = _atlases.find(atlasName);
   if (found != _atlases.end()) {
      return found->second.get();
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
optional<TileMap::TileMapLayer*> TileMap::addLayer(SpriteAtlas *atlas) {
   if (!atlas){
      Logger::error() << "TileMap::addLayer - cannot add null atlas!" << endl;
      return {};
   }
   auto retval = _layers.emplace(getFirstEmptyLayerIndex(), make_unique<TileMapLayer>(*this, atlas));
   return retval.first->second.get();
}

/////////////////////////////////////////////////////////////////////////////////////////
TileMap::LayerIndex TileMap::getFirstEmptyLayerIndex() {
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
optional<TileMap::TileMapLayer*> TileMap::getLayer(ReyEngine::TileMap::LayerIndex idx) {
   auto found = _layers.find(idx);
   if (found == _layers.end()){
      Logger::error() << "TileMap layer index " + to_string(idx) + " not found!" << endl;
      return {};
   }
   return found->second.get();
}

/////////////////////////////////////////////////////////////////////////////////////////
TileMap::TileCoord TileMap::getCell(const Pos<R_FLOAT>& localPos) const {
   auto vec2= getRect().toSizeRect().getSubRectCoord({(float)_tileSize.x, (float)_tileSize.y}, localPos);
   return {(int)vec2.x, (int)vec2.y};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> TileMap::getCellPos(const TileCoord& coord) const {
   auto x = coord.x * _tileSize.x;
   auto y = coord.y * _tileSize.y;
   return {(float)x, (float)y};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> TileMap::getCellPosCenter(const TileCoord& coord) const {
   auto pos = getCellPos(coord);
   return pos + getTileSize()/2;
}

/////////////////////////////////////////////////////////////////////////////////////////
int TileMap::getCellIndex(const TileCoord& coord) const {
   return getRect().getSubRectIndex(getTileSize(), getCellPos(coord));
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<R_FLOAT> TileMap::getCellRect(const TileCoord& coord) const {
   return {getCellPos(coord), getTileSize()};
}

////////////////////////////////////////////////////////////////////////////////////////
void TileMap::setTileSize(const Size<int>& size) {
   _tileSize.x = size.x;
   _tileSize.y = size.y;
   redraw();
}

/////////////////////////////////////////////////////////////////////////////////////////
Widget* TileMap::_unhandled_input(const InputEvent& event) {
   auto mouse = event.isMouse();
   if (mouse && mouse.value()->isInside()) {
      switch (event.eventId) {
         case InputEventMouseMotion::getUniqueEventId(): {
            auto cellPos = getCell(mouse.value()->getLocalPos());
            float dt = (float)cellPos.x - ((float)mouse.value()->getLocalPos().x / (float)_tileSize.x);
            if (cellPos != currentHover) {
               currentHover = cellPos;
               EventTileMapCellHovered _event(this, currentHover.value());
               publish(_event);
               return this;
            }
         break;
         }
         case InputEventMouseButton::getUniqueEventId():
            auto mbEvent = event.toEvent<InputEventMouseButton>();
            if (mbEvent.isDown && mbEvent.button == InputInterface::MouseButton::LEFT){
               EventTileMapCellClicked _event(this, currentHover.value());
               publish(_event);
               return this;
            }
         break;
      }
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_on_rect_changed() {
   _renderTarget.setSize(getSize());
   _needsRedraw = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
optional<TileMap::TileIndex> TileMap::TileMapLayer::getTileIndex(const TileCoord &pos) {
   try {
      return tiles.at(pos.x).at(pos.y);
   } catch (const out_of_range& e) {
      Logger::debug() << "Tile layer " << atlas->getPath() << " has no index at coordinates " << pos << endl;
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
   tileMap._needsRedraw = true;
}


/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::TileMapLayer::setTileByIndex(const TileCoord& coords, TileIndex index) {
   tiles[coords.x][coords.y] = index;
   tileMap._needsRedraw = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::TileMapLayer::setTileByCoords(const TileCoord& target, const TileCoord& source) {
   tiles[target.x][target.y] = source.x * atlas->_columnCount * source.x + source.y;
   tileMap._needsRedraw = true;
}