#pragma once
#include "Canvas.h"
#include <map>
namespace ReyEngine {
   class TileMap : public Canvas {
   public:
      struct TileCoord : public Vec2<int> {
           TileCoord(int x, int y): Vec2(x,y){}
//         TileCoord(int x, int y): x(x),y(y){}
//         inline explicit operator bool() const {return x || y;}
//         inline TileCoord operator+(const TileCoord& rhs) const {TileCoord val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
//         inline TileCoord operator-(const TileCoord& rhs) const {TileCoord val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
//         inline TileCoord& operator+=(const TileCoord& rhs){x += rhs.x; y += rhs.y; return *this;}
//         inline TileCoord& operator-=(const TileCoord& rhs){x -= rhs.x; y -= rhs.y; return *this;}
//         inline TileCoord& operator*=(const TileCoord& rhs){x *= rhs.x; y *= rhs.y; return *this;}
//         inline TileCoord& operator/=(const TileCoord& rhs){x /= rhs.x; y /= rhs.y; return *this;}
//         inline TileCoord& operator=(const TileCoord& rhs){x = rhs.x; y=rhs.y; return *this;}
//         inline bool operator==(const TileCoord& rhs){return x==rhs.x && y==rhs.y;}
//         inline bool operator!=(const TileCoord& rhs){return x!=rhs.x || y!=rhs.y;}
//         inline TileCoord& operator-(){x = -x; y =-y; return *this;}
////         inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(2, s);};
////         std::ostream& operator<<(std::ostream& os) const {os << Vec<T>::toString(); return os;}
//         friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
//         int x;
//         int y;
      };
      using TileIndex = uint64_t;
      using LayerIndex = uint64_t;

      struct SpriteAtlas {
         SpriteAtlas(const FileSystem::File& file)
         : texture(file)
         , _filePath(file)
         {
            setTileSize(_tileSize);
         }
         SpriteAtlas(SpriteAtlas&& other)
         : texture(std::move(other.texture))
         , _filePath(std::move(other._filePath))
         , _tileSize(std::move(other._tileSize))
         , rowCount(std::move(other.rowCount))
         , columnCount(std::move(other.columnCount))
         {}
         inline void setTileSize(Size<int> size){
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
         inline const FileSystem::File getFilePath() const {return _filePath;}
      private:
         ReyTexture texture;
         FileSystem::File _filePath;
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

//   struct CellDataProperty : public Property<std::map<Vec2<int>, CellData>>{
//         using Property<std::map<Vec2<int>, CellData>>::operator=;
//      CellDataProperty(const std::string& instanceName)
//         : Property(instanceName, PropertyTypes::TileMapData, std::map<Vec2<int>, CellData>())
//         {}
//         std::string toString() const override {throw std::runtime_error("not implemented"); return "";}
//         std::map<Vec2<int>, CellData> fromString(const std::string& str) override { }
//      };
//
//      struct TileMapLayerProperty : public Property<TileMapLayer>{
//         using Property<TileMapLayer>::operator=;
//         TileMapLayerProperty(const std::string& instanceName)
//         : Property(instanceName, PropertyTypes::TileMapLayer, TileMapLayer())
//         {}
//         std::string toString() const override {throw std::runtime_error("not implemented"); return "";}
//         std::map<Vec2<int>, CellData> fromString(const std::string& str) override { }
//      };

//      using TileMapData = std::map<uint64_t, ReyTexture>;
//      struct TileMapDataProperty : public Property<>{
//         using Property<std::map<Vec2<int>, TileMapData>>::operator=;
//         TileMapDataProperty(const std::string& instanceName)
//               : Property(instanceName, PropertyTypes::TileMapLayer, std::map<Vec2<int>, TileMapData>())
//         {}
//         std::string toString() const override {throw std::runtime_error("not implemented"); return "";}
//         std::map<Vec2<int>, TileMapData> fromString(const std::string& str) override { }
//      };


      enum class GridType {SQUARE, SQUARE_OFFSET, HEX};
      struct GridTypeProperty : public EnumProperty<GridType, 3>{
         GridTypeProperty(const std::string& instanceName,  GridType defaultvalue)
         : EnumProperty<GridType, 3>(instanceName, defaultvalue){}
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

      REYENGINE_OBJECT(TileMap, Canvas)
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
      void setGridSize(const Size<int>&);
   protected:
      void render() const override;
//      inline void renderBegin(ReyEngine::Pos<double>&) override;
//      inline void renderEnd() override;
      virtual Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void registerProperties() override;
      void _init() override;
      void _on_rect_changed() override;
   private:
      void updateAllLayers();
      bool _ready = false;
      BoolProperty _showGrid;
      GridTypeProperty _gridType;
      IntProperty _gridHeight;
      IntProperty _gridWidth;
      std::map<LayerIndex, TileMapLayer> _layers;
      LayerIndex getNextLayerIndex();
      TileCoord currentHover;
//      RenderTarget _renderTarget;
   };
}
