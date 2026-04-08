#include "TileMap.h"
#include "Application.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_init() {
   _showGrid = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render2DEnd(RenderContext&) {
   if (_needsRedraw) {
      _needsRedraw = false;
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::render2D(RenderContext&) const {
   // if (!_needsRedraw) return;
   // ClearBackground(Colors::none);
   //draw all tiles in the layer
   for (auto& layer: _layers) {
      for (auto& [x, yMap]: layer.second->tiles) {
         for (auto& [y, atlasCoords]: yMap) {
            auto pos = getCellPos({x, y});
            auto srcRect = layer.second->atlas->getTile(atlasCoords);
            auto dstRect = Rect<R_FLOAT>(pos, {(R_FLOAT) _tileSize.x, (R_FLOAT) _tileSize.y});
            auto& tex = layer.second->atlas->texture;
            drawTexture(tex, srcRect, dstRect, Colors::none);
         }
      }
   }

   //draw grid on top of tiles
   if (_showGrid) {
      drawRectangleLines(getSizeRect(), 1.0, theme->background.colorSecondary);
      switch (_gridType) {
         case GridType::SQUARE: {
            auto xLineCount = getWidth() / (float) _tileSize.x + 1;
            auto yLineCount = getHeight() / (float) _tileSize.y + 1;
            for (int x = 1; x < (int) xLineCount; x++) {
               auto _x = x * _tileSize.x;
               auto linex = Line<R_FLOAT>({(float) _x, 0}, {(float) _x, getHeight()});
               drawLine(linex, 1, theme->background.colorSecondary);
               for (int y = 1; y < yLineCount + 1; y++) {
                  auto _y = y * _tileSize.y;
                  auto liney = Line<R_FLOAT>({0, (float) _y}, {getWidth(), (float) _y});
                  drawLine(liney, 1, theme->background.colorSecondary);
               }
            }
            break;
         }
         case GridType::HEX:
            break;
         case GridType::SQUARE_OFFSET:
            break;
      }
   }
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
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
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
   return pos + Pos<R_FLOAT>(getTileSize()/2);
}

/////////////////////////////////////////////////////////////////////////////////////////
int TileMap::getCellIndex(const TileCoord& coord) const {
   return getRect().getSubRectIndex(Size<R_FLOAT>(getTileSize()), getCellPos(coord));
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<R_FLOAT> TileMap::getCellRect(const TileCoord& coord) const {
   return {getCellPos(coord), Size<R_FLOAT>(getTileSize())};
}

////////////////////////////////////////////////////////////////////////////////////////
void TileMap::setTileSize(const Size<int>& size) {
   _tileSize.x = size.x;
   _tileSize.y = size.y;
   redraw();
}
////////////////////////////////////////////////////////////////////////////////////////
std::optional<float> TileMap::SpriteAtlas::getNavWeight(const TileCoord& coord) const {
   const auto found = _weights.find(coord);
   if (found == _weights.end()) return {};
   return found->second;
}

////////////////////////////////////////////////////////////////////////////////////////
std::optional<float> TileMap::getTotalNavWeight(const TileCoord& coords) const {
   bool found = false;
   float totalWeight = 1;
   for (const auto& [coord, layer] : _layers) {
      if (auto weight = layer->getAtlas().value()->getNavWeight(coords)) {
         totalWeight *= weight.value();
         found = true;
      }
   }
   return found ? totalWeight : optional<float>();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled TileMap::_unhandled_input(const InputEvent& event) {
   auto mouse = event.isMouse();
   if (mouse && mouse.value()->isInside()) {
      switch (event.eventId) {
         case InputEventMouseHover::ID: {
            auto cellPos = getCell(mouse.value()->getLocalPos());
            if (cellPos != currentHover) {
               currentHover = cellPos;
               if (!_on_hovered(cellPos)) {
                  EventTileMapCellHovered _event(this, currentHover.value());
                  publish(_event);
               }
               return this;
            }
         break;
         }
         case InputEventMouseButton::ID:
            auto& mbEvent = event.toEvent<InputEventMouseButton>();
            if (mbEvent.isDown && mbEvent.button == InputInterface::MouseButton::LEFT){
               if (!_on_clicked(currentHover.value())) {
                  EventTileMapCellClicked _event(this, currentHover.value());
                  publish(_event);
               }
               return this;
            }
         break;
      }
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_on_rect_changed() {
   _needsRedraw = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TileMap::_on_tiles_changed() {
   if (_autoResize) {
      Rect<float> combined;
      bool first = true;
      for (const auto& [idx, layer] : _layers) {
         auto layerBoundingBox = layer->getBoundingBox();
         if (layerBoundingBox.width <= 0 || layerBoundingBox.height <= 0) continue;
         if (first) {
            combined = layerBoundingBox;
            first = false;
         } else {
            combined = combined.combine(layerBoundingBox);
         }
      }
      if (!first) {
         setRect(combined);
      }
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
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
void TileMap::TileMapLayer::setTileAtCoords(const TileCoord& target, const TileCoord& source) {
   tiles[target.x][target.y] = source;
   tileMap._on_tiles_changed();
   tileMap._needsRedraw = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<TileMap::TileCoord> TileMap::TileMapLayer::getTileAtCoords(const TileCoord& src) {
   const auto foundX = tiles.find(src.x);
   if (foundX == tiles.end()) return {};
   const auto foundY = foundX->second.find(src.y);
   if (foundY == foundX->second.end()) return {};
   return foundY->second;
}

/////////////////////////////////////////////////////////////////////////////////////////
Rect<float> TileMap::TileMapLayer::getBoundingBox() const {
   // we (we? ha! YOU) can improve this algorithm by making it create a rect from 0,0 that only includes each new tile,
   // then comparing that rect with our current rect, and expanding if necessary. We still would need to do a
   // full recalculation. Possibly we could track the cornery-est tiles and use that to determine if we should
   // shrink to speed that up too.
   if (tiles.empty()) return {};

   int minX = tiles.begin()->first;
   int maxX = tiles.rbegin()->first;
   int minY = std::numeric_limits<int>::max();
   int maxY = std::numeric_limits<int>::min();

   for (const auto& [x, yMap] : tiles) {
      if (!yMap.empty()) {
         minY = std::min(minY, yMap.begin()->first);
         maxY = std::max(maxY, yMap.rbegin()->first);
      }
   }

   auto tileSize = tileMap.getTileSize();
   float posX = minX * tileSize.x;
   float posY = minY * tileSize.y;
   float width = (maxX - minX + 1) * tileSize.x;
   float height = (maxY - minY + 1) * tileSize.y;
   return {{posX, posY}, {width, height}};
}
