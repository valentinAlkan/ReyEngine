#pragma once
#include "Canvas.h"
#include <map>
namespace ReyEngine {
   class TileMap : public Canvas {
   public:
      struct TileCoord : public Vec2<int>{
         TileCoord(int x, int y): Vec2<int>(x,y){}
      };
      using TileIndex = uint64_t;
      using LayerIndex = uint64_t;

      struct SpriteAtlas {
         SpriteAtlas(const FileSystem::File& file)
         : texture(file)
         , filePath(file)
         {}
         SpriteAtlas(SpriteAtlas&& other)
         : texture(std::move(other.texture))
         , filePath(std::move(other.filePath))
         {}
         inline void setTileSize(Size<int> size){tileSize = size;}
         FileSystem::File filePath;
         ReyTexture texture;
         Size<int> tileSize = {32,32};
      };

      struct TileMapLayer{
         TileMapLayer(const FileSystem::File& file): atlas(file){}
         TileMapLayer(TileMapLayer&& other) noexcept: atlas(std::move(other.atlas)){}
         void setTileIndex(const TileCoord&, TileIndex);
         std::optional<TileIndex> getTileIndex(const TileCoord& pos); //slow
         inline SpriteAtlas& getAtlas(){return atlas;}
      protected:
         //x, y
         std::map<int, std::map<int, TileIndex>> tiles;
         SpriteAtlas atlas;
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

      REYENGINE_OBJECT(TileMap, Canvas)
      , PROPERTY_DECLARE(_showGrid, true)
      , PROPERTY_DECLARE(_gridType, GridType::SQUARE)
      , PROPERTY_DECLARE(_gridHeight, 32)
      , PROPERTY_DECLARE(_gridWidth, 32)
      {}
   public:
      std::optional<LayerIndex> addLayer(const FileSystem::File&);
      TileMapLayer& getLayer(LayerIndex);
      TileCoord getCoord(Pos<int>) const;
      Pos<int> getPos(TileCoord) const;
      void setGridSize(Size<int>);
   protected:
      void render() const override;
//      inline void renderBegin(ReyEngine::Pos<double>&) override;
//      inline void renderEnd() override;
      void registerProperties() override;
      void _init() override;
      void _on_rect_changed() override;
   private:
      bool _ready = false;
      BoolProperty _showGrid;
      GridTypeProperty _gridType;
      IntProperty _gridHeight;
      IntProperty _gridWidth;
      std::map<LayerIndex, TileMapLayer> _layers;
      LayerIndex getNextLayerIndex();
//      RenderTarget _renderTarget;
   };
}
