#pragma once
#include "BaseWidget.h"
#include <map>
namespace ReyEngine {
   class TileMap : public BaseWidget {

      struct SpriteAtlas {
         SpriteAtlas(const FileSystem::File& file): texture(file){}
         SpriteAtlas(SpriteAtlas&& other): texture(other.texture), filePath(other.filePath){}
         FileSystem::File filePath;
         ReyTexture texture;
      };

      using TileCoord = Vec2<int>;
      using TileIndex = uint64_t;
      using LayerIndex = uint64_t;
      struct TileMapLayer{
         std::map<TileCoord, TileIndex> tiles;
         SpriteAtlas atlas;
         TileMapLayer(TileMapLayer&& other) noexcept
         : atlas(other)
         {
         };
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

      REYENGINE_OBJECT(TileMap, BaseWidget)
      , PROPERTY_DECLARE(_showGrid, true)
      , PROPERTY_DECLARE(_gridType, GridType::SQUARE)
      {}
   public:
      std::optional<LayerIndex> addTexture(const FileSystem::File&);
   protected:
      void render() const override;
      void registerProperties() override;
   private:
      BoolProperty _showGrid;
      GridTypeProperty _gridType;
      std::map<LayerIndex, TileMapLayer> _layers;
      LayerIndex getNextLayerIndex();
   };
}
