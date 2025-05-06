#pragma once
#include <map>
#include "Canvas.h"

namespace ReyEngine {
   class TileMap : public Canvas {
   public:
      REYENGINE_OBJECT(TileMap)
      struct TileCoord : public Vec2<int> {
           constexpr TileCoord(): Vec2(){}
           constexpr TileCoord(int x, int y): Vec2(x,y){}
           constexpr TileCoord(const Vec2<int>& other): Vec2(other){}
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
            _rowCount = texture.size.x / _tileSize.x;
            _columnCount = texture.size.y / _tileSize.y;
            _needsUpdate = true;
         }
//         [[nodiscard]] inline TileIndex getTileIndex(const Pos<int>& pos) const {
//            //find the index of the tile at the position
//            auto rect = Rect<int>(texture.size);
//            return rect.getSubRectIndex(_tileSize, pos);
//         }
         [[nodiscard]] inline Rect<R_FLOAT> getTile(const TileCoord coords) const {
            Pos<R_FLOAT> src = {(R_FLOAT) _tileSize.x * coords.x + (_padding.x * coords.x + 1) + _offset.x, (R_FLOAT) _tileSize.y * coords.y + (_padding.y * coords.y + 1) + _offset.y};
            return {src, _tileSize};
         }
         [[nodiscard]] inline Size<int> getTileSize() const {return _tileSize;}
         [[nodiscard]] inline Size<int> getSheetSize() const {return {texture.size.x, texture.size.y};}
         inline const FileSystem::File getFile() const {return texture.getPath();}
         std::string getPath() {return texture.getPath();}
      protected:
         int _columnCount; //cached
         int _rowCount; //cached
      private:
         ReyTexture texture;
         Size<R_FLOAT> _tileSize = {32, 32};
         Pos<R_FLOAT> _offset; //x,y offset of first tile
         Size<R_FLOAT> _padding; // lines of spacing between each tile
         bool _needsUpdate = true;
      friend class TileMap;
      };
      struct TileMapLayer{
         TileMapLayer(TileMap& tileMap, SpriteAtlas* atlas): tileMap(tileMap), atlas(atlas){}
         inline std::optional<SpriteAtlas*> getAtlas(){return atlas;}
//         void setTileByIndex(const TileCoord&, TileIndex);
         void setTileByCoords(const TileCoord& src, const TileCoord& target);
//         std::optional<TileIndex> getTileIndex(const TileCoord& pos); //slow
         void removeTileIndex(const TileCoord& pos);
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
   public:
      TileMap(const Size<int>& tileSize): _tileSize(tileSize){}
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
   protected:
      virtual Handled _on_hovered(const TileCoord&){return false;}
      virtual Handled _on_clicked(const TileCoord&){return false;}
      void render2DBegin() override;
      void render2D() const override;
      void render2DEnd() override;
      void _init();
      Widget* _unhandled_input(const InputEvent&) override;
      void _on_rect_changed() override;
      std::map<LayerIndex, std::unique_ptr<TileMapLayer>> _layers;
      std::map<std::string, std::unique_ptr<SpriteAtlas>> _atlases;
      bool _showGrid;

private:
      LayerIndex getFirstEmptyLayerIndex();
      GridType _gridType = GridType::SQUARE;
      Size<int> _tileSize;
      bool _needsRedraw = false;
      std::optional<TileCoord> currentHover;

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
}
