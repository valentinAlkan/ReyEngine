#pragma once
#include <map>
#include <tuple>
#include "Canvas.h"

namespace ReyEngine {
   class TileMap : public Widget {
   public:
      enum class NeighborType{ORTHOGONAL, DIAGONAL}; //orthogonal neighbors are only NSEW neighbors, diagonal includes NE, NW, SE and SW neighbors too
      REYENGINE_OBJECT(TileMap)
      struct TileCoord : public Vec2<int> {
         constexpr TileCoord(): Vec2(){}
         constexpr TileCoord(int x, int y): Vec2(x,y){}
         constexpr TileCoord(const Vec2<int>& other): Vec2(other){}
         constexpr TileCoord(const TileCoord& other) : Vec2<int>(other.x, other.y) {}
         constexpr TileCoord(TileCoord&& other) noexcept : Vec2<int>(other.x, other.y) {}
         constexpr TileCoord& operator=(const TileCoord& other) {
            Vec2<int>::x = other.x;
            Vec2<int>::y = other.y;
            return *this;
         }
         constexpr TileCoord& operator=(TileCoord&& other) noexcept {
            Vec2<int>::x = other.x;
            Vec2<int>::y = other.y;
            return *this;
         }
         constexpr bool operator<(const TileCoord& other) const {
            if (y != other.y) return y < other.y;
            return x < other.x;
         }
      };
      using LayerIndex = size_t;

      struct SpriteAtlas {
      protected:
         SpriteAtlas(
            const FileSystem::File& file,
            const Size<int>& tileSize,
            const Pos<int>& offset,
            const Size<int>& padding)
         : texture(file)
         , _tileSize(tileSize)
         , _offset(offset)
         , _padding(padding)
         {
            setTileSize(_tileSize);
         }
         SpriteAtlas(SpriteAtlas&& other)
         : texture(std::move(other.texture))
         , _tileSize(other._tileSize)
         , _offset(other._offset)
         , _padding(other._padding)
         {}
      public:
         inline void setTileSize(const Size<R_FLOAT>& size){
            _tileSize = size;
            _rowCount = texture.size().x / _tileSize.x;
            _columnCount = texture.size().y / _tileSize.y;
            _needsUpdate = true;
         }
         std::optional<float> getNavWeight(const TileCoord& coord) const;
         inline void setNavWeight(const TileCoord& coord, float weight){_weights[coord] = weight;}
         [[nodiscard]] inline Rect<R_FLOAT> getTile(const TileCoord& coords) const {
            Pos<R_FLOAT> src = {(R_FLOAT) _tileSize.x * coords.x + (_padding.x * coords.x + 1) + _offset.x, (R_FLOAT) _tileSize.y * coords.y + (_padding.y * coords.y + 1) + _offset.y};
            return {src, _tileSize};
         }
         [[nodiscard]] inline Size<int> getTileSize() const {return _tileSize;}
         [[nodiscard]] inline Size<int> getSheetSize() const {return {texture.size().x, texture.size().y};}
//         inline const FileSystem::File getFile() const {return texture.getPath();}
//         std::string getPath() {return texture.getPath();}
      protected:
         int _columnCount; //cached
         int _rowCount; //cached
      private:
         ReyTexture texture;
         Size<R_FLOAT> _tileSize = {32, 32};
         Pos<R_FLOAT> _offset; //x,y offset of first tile
         Size<R_FLOAT> _padding; // lines of spacing between each tile
         bool _needsUpdate = true;
         std::map<TileCoord, float> _weights; //optional weighting
      friend class TileMap;
      };
      struct TileMapLayer{
         TileMapLayer(TileMap& tileMap, SpriteAtlas* atlas): tileMap(tileMap), atlas(atlas){}
         inline std::optional<SpriteAtlas*> getAtlas(){return atlas;}
         void setTileAtCoords(const TileCoord& target, const TileCoord& src);
         std::optional<TileCoord> getTileAtCoords(const TileCoord& src);
         void removeTileIndex(const TileCoord& pos);
         [[nodiscard]] Rect<float> getBoundingBox() const; //determine the size of the bounding box that encapsulates every set tile in this layer
      protected:
         //x, y
         std::map<int, std::map<int, TileCoord>> tiles;
      private:
         SpriteAtlas* atlas = nullptr;
         TileMap& tileMap;
         friend class TileMap;
      };

      enum class GridType {SQUARE, SQUARE_OFFSET, HEX};
      EVENT_ARGS(EventTileMapCellClicked, 1354659879, const TileCoord& cellCoord)
      , cellCoord(cellCoord)
      {}
         const TileCoord& cellCoord;
      };

      EVENT_ARGS(EventTileMapCellHovered, 1354659880, const TileCoord& cellCoord)
      , cellCoord(cellCoord)
      {}
         const TileCoord& cellCoord;
      };

      TileMap(const Size<int>& tileSize): _tileSize(tileSize) {}
      TileMap(int tileWidth, int tileheight): _tileSize(Size<int>(tileWidth, tileheight)){}
      std::optional<SpriteAtlas*> addAtlas(
            const std::string& atlasName,
            const FileSystem::File& file,
            const Size<int>& tileSize,
            const Pos<int>& offset,
            const Size<int>& padding);
      std::optional<SpriteAtlas*> getAtlas(const std::string& atlasName);
      std::optional<TileMapLayer*> addLayer(SpriteAtlas* atlas);
      std::optional<TileMapLayer*> getLayer(LayerIndex);
      [[nodiscard]] TileCoord getCell(const Pos<R_FLOAT>&) const;
      [[nodiscard]] Pos<R_FLOAT> getCellPos(const TileCoord&) const;
      [[nodiscard]] Rect<R_FLOAT> getCellRect(const TileCoord&) const;
      [[nodiscard]] Pos<R_FLOAT> getCellPosCenter(const TileCoord&) const;
      [[nodiscard]] TileCoord getTileCoords(const Pos<R_FLOAT>& pos) const {
         auto coords = getRect().getSubRectCoord(getTileSize(), pos);
         return {coords.x, coords.y};
      }
      [[nodiscard]] int getCellIndex(const TileCoord&) const;
      ///Set the size of each tile
      void setTileSize(const Size<int>&);
      [[nodiscard]] Size<int> getTileSize() const {return _tileSize;}
      void redraw(){_needsRedraw = true;}
      std::optional<float> getTotalNavWeight(const TileCoord&) const;
      std::vector<TileCoord> getTileNeighbors(const TileCoord& coords) const {
         std::vector<TileCoord> neighbors;
         neighbors.reserve(_neighborType == NeighborType::DIAGONAL ? 8 : 4);
         auto x = coords.x;
         auto y = coords.y;
         std::vector<std::pair<int, int>> offsets = {
            {x, y - 1}, // N
            {x, y + 1}, // S
            {x + 1, y}, // E
            {x - 1, y}  // W
         };
         if (_neighborType == NeighborType::DIAGONAL) {
            offsets.push_back({x + 1, y - 1}); // NE
            offsets.push_back({x - 1, y - 1}); // NW
            offsets.push_back({x + 1, y + 1}); // SE
            offsets.push_back({x - 1, y + 1}); // SW
         }
         return neighbors;
      }
   protected:
      virtual Handled _on_hovered(const TileCoord&){return nullptr;}
      virtual Handled _on_clicked(const TileCoord&){return nullptr;}
      // void render2DBegin(RenderContext&) override;
      void render2D(RenderContext&) const override;
      void render2DEnd(RenderContext&) override;
      void _init();
      Handled _unhandled_input(const InputEvent&) override;
      void _on_rect_changed() override;
      void _on_tiles_changed();
      std::map<LayerIndex, std::unique_ptr<TileMapLayer>> _layers;
      std::map<std::string, std::unique_ptr<SpriteAtlas>> _atlases;
      bool _showGrid = false;
      bool _autoResize = true; //when adding tiles, the size of the tilemap expands to include them

private:
      LayerIndex getFirstEmptyLayerIndex();
      GridType _gridType = GridType::SQUARE;
      Size<int> _tileSize;
      bool _needsRedraw = false;
      std::optional<TileCoord> currentHover;
      NeighborType _neighborType = NeighborType::DIAGONAL;

      /////////////////////////////////////////////////////////////////////////////////////////
      // Iterator class for layers
//      class iterator : public std::iterator<std::forward_iterator_tag, std::string> {
//      public:
//         iterator(std::optional<TileMap*> tileMap = std::nullopt): _tileMap(tileMap){}
//         const TileMapLayer& operator*() const {
//            return _tileMap.value()->getLayer(layerNo);
//         }
//         iterator& operator++() {
//            layerNo++;
//            return *this;
//         }
//
//         bool operator!=(const iterator& other) const {
//            if (!_tileMap) return false;
//            if (!other._tileMap) return false;
//            return layerNo != other.layerNo;
//         }
//
//         [[nodiscard]] size_t getCurrentRowNo() const{return layerNo;}
//      private:
//         std::map<LayerIndex, TileMapLayer>::iterator it;
//         size_t layerNo = std::numeric_limits<size_t>::max();
//         std::optional<TileMap*> _tileMap;
//      };
//
//      iterator begin() {
//         auto it = iterator(this);
//         return it;
//      }
//      [[nodiscard]] static iterator end() { return {};}
//      friend class iterator;
   };

   // Structured binding support for TileCoord
   template<std::size_t I>
   constexpr int get(const TileMap::TileCoord& t) {
      if constexpr (I == 0) return t.x;
      else if constexpr (I == 1) return t.y;
   }
}

namespace std {
   template<>
   struct tuple_size<ReyEngine::TileMap::TileCoord> : std::integral_constant<std::size_t, 2> {};

   template<std::size_t I>
   struct tuple_element<I, ReyEngine::TileMap::TileCoord> {
      using type = int;
   };
}
