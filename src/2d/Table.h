#pragma once
#include "ScrollArea.h"

namespace ReyEngine {
   class TableItem : public Internal::Drawable2D, public MetaDataInterface {
      std::string text;
      std::shared_ptr<Theme> overrideTheme;
   };
   class Table : public ScrollArea {
   public:
      REYENGINE_OBJECT(Table)
      template <typename T>
      void setData(const Pos<size_t>& cellPos, const std::string& metaName, const T& data){
         auto it = _data.try_emplace(cellPos.x).first->second.try_emplace(cellPos.y);
         auto& ptr = it.first->second;
         if (!ptr){
            //add the child
            auto child = make_child<TableItem>(this, std::format("item {},{}", cellPos.x, cellPos.y));
            auto c = child->as<TypeNode>;
         }
         ptr->setMetaData(metaName, data);
         return;
      }
      template <typename T>
      std::optional<T*> getData(const Pos<size_t>& cellPos, const std::string& metaName) const {
         auto foundX = _data.find(cellPos.x);
         if (foundX == _data.end()) return {};
         auto foundY = foundX->second.find(cellPos.y);
         if (foundY == foundX->second.end()) return {};
         if (auto metaDataAble = foundY->second->tag<MetaDataInterface>()){
            return metaDataAble.value()->getMetaData<T>(metaName);
         }
         return {};
      }
      void setWidgetAsData(const std::shared_ptr<Widget>& w){

      }
      std::map<size_t, std::map<size_t, TypeNode*>> _data;
      //std::vector<std::vector<TableItem*>> _cache; //todo: for fast access
   };
}
