#pragma once
#include "Canvas.h"
#include <map>

namespace ReyEngine {
   class TileMap : public BaseWidget {
   public:
      struct TileCoord : public Vec2<int> {
           TileCoord(int x, int y): Vec2(x,y){}
           TileCoord(const Vec2<int>& other): Vec2(other){}
      };
      using TileIndex = uint64_t;
      using LayerIndex = uint64_t;

      struct SpriteAtlas {
         SpriteAtlas(const FileSystem::File& file)
         : texture(file)
         , _file(file)
         {
            setTileSize(_tileSize);
         }
         SpriteAtlas(SpriteAtlas&& other)
         : texture(std::move(other.texture))
         , _file(other._file)
         , _tileSize(other._tileSize)
         , rowCount(other.rowCount)
         , columnCount(other.columnCount)
         {}
         inline void setTileSize(const Size<int>& size){
            _tileSize = size;
            rowCount = texture.size.x / _tileSize.x;
            columnCount = texture.size.y / _tileSize.y;
            _needsUpdate = true;
         }
         inline std::optional<TileIndex> getTileIndex(const Pos<int>& pos) const {
            //find the index of the tile at the position
            auto rect = Rect<int>(texture.size);
            if (!rect.isInside(pos)) return std::nullopt;
            return rect.getSubRectIndex(_tileSize, pos);
         }
         inline std::optional<Rect<int>> getTile(const LayerIndex index) const {
            //find the rect of the tile at the index
            auto rect = Rect<int>(texture.size);
//            if (!rect.isInside(pos)) return std::nullopt;
            return rect.getSubRect(_tileSize, index);
         }
         inline Size<int> getTileSize() const {return _tileSize;}
         inline const FileSystem::File getFile() const {return _file;}
      private:
         ReyTexture texture;
         FileSystem::File _file;
         int rowCount = 0;
         int columnCount = 0;
         Size<int> _tileSize = {32, 32};
         bool _needsUpdate = true;
      friend class TileMap;
      };

      struct TileMapLayer{
         TileMapLayer(const FileSystem::File& file): atlas(file){}
         TileMapLayer(TileMapLayer&& other) noexcept: atlas(std::move(other.atlas)){}
         void setTileIndex(const TileCoord&, TileIndex);
         std::optional<TileIndex> getTileIndex(const TileCoord& pos); //slow
         void removeTileIndex(const TileCoord& pos);
         inline SpriteAtlas& getAtlas(){return atlas;}
         bool needsUpdate() const {return _needsUpdate || atlas._needsUpdate;}
      protected:
         //x, y
         std::map<int, std::map<int, TileIndex>> tiles;
         SpriteAtlas atlas;
      private:
         bool _needsUpdate = true;
         friend class TileMap;
      };

      enum class GridType {SQUARE, SQUARE_OFFSET, HEX};
      struct GridTypeProperty : public EnumProperty<GridType, 3>{
         GridTypeProperty(const std::string& instanceName,  GridType defaultvalue)
         : EnumProperty<GridType, 3>(instanceName, std::move(defaultvalue)){}
         const EnumPair<GridType, 3>& getDict() const override {return dict;}
         static constexpr EnumPair<GridType, 3> dict = {
         ENUM_PAIR_DECLARE(GridType, SQUARE),
         ENUM_PAIR_DECLARE(GridType, SQUARE_OFFSET),
         ENUM_PAIR_DECLARE(GridType, HEX),
         };
         void registerProperties() override {}
      };

      struct EventTileMapCellClicked : public Event<EventTileMapCellClicked>{
         EVENT_CTOR_SIMPLE(EventTileMapCellClicked, Event<EventTileMapCellClicked>, const TileCoord& cellCoord, const Rect<int>& cellRect), cellCoord(cellCoord), cellRect(cellRect){}
         const TileCoord& cellCoord;
         const Rect<int>& cellRect;
      };

      struct EventTileMapCellHovered : public Event<EventTileMapCellHovered>{
         EVENT_CTOR_SIMPLE(EventTileMapCellHovered, Event<EventTileMapCellHovered>, const TileCoord& cellCoord, const Rect<int>& cellRect), cellCoord(cellCoord), cellRect(cellRect){}
         const TileCoord& cellCoord;
         const Rect<int>& cellRect;
      };

      REYENGINE_OBJECT_BUILD_ONLY(TileMap, BaseWidget)
      , PROPERTY_DECLARE(_showGrid, true)
      , PROPERTY_DECLARE(_gridType, GridType::SQUARE)
      , PROPERTY_DECLARE(_gridHeight, 32)
      , PROPERTY_DECLARE(_gridWidth, 32)
      , currentHover(-1,-1)
      {}
   public:
      std::optional<LayerIndex> addLayer(const FileSystem::File&);
      TileMapLayer& getLayer(LayerIndex);
      TileCoord getCell(const Pos<int>&) const;
      Pos<int> getCellPos(const TileCoord&) const;
      Rect<int> getCellRect(const TileCoord&) const;
      Pos<int> getCellPosCenter(const TileCoord&) const;
      int getCellIndex(const TileCoord&) const;
      ///Set the size of each tile
      void setTileSize(const Size<int>&);
      Size<int> getTileSize() const {return {_gridWidth, _gridHeight};}
   protected:
      void render() const override;
      virtual Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void registerProperties() override;
   protected:
      std::map<LayerIndex, TileMapLayer> _layers;
   private:
      void updateAllLayers();
      BoolProperty _showGrid;
      GridTypeProperty _gridType;
      IntProperty _gridHeight;
      IntProperty _gridWidth;
      LayerIndex getFirstEmptyLayerIndex();
      TileCoord currentHover;

      // Iterator class for rows of csv data
      class iterator : public std::iterator<std::forward_iterator_tag, std::string> {
      public:
         iterator(std::optional<std::reference_wrapper<TileMap>> tileMap = std::nullopt): _tileMap(tileMap){}
         const TileMapLayer& operator*() const {
            return _tileMap.value().get().getLayer(layerNo);
         }
         iterator& operator++() {
            layerNo++;
            return *this;
         }

         bool operator!=(const iterator& other) const {
            if (!_tileMap) return false;
            if (!other._tileMap) return false;
            return layerNo != other.layerNo;
         }

         size_t getCurrentRowNo(){return layerNo;}
      private:
         std::map<LayerIndex, TileMapLayer>::iterator it;
         size_t layerNo = -1;
         std::optional<std::reference_wrapper<TileMap>> _tileMap;
      };

      iterator begin() {
         auto it = iterator(std::ref(*this));
         return it;
      }
      iterator end() const { return {};}
      friend class iterator;
   };
}
