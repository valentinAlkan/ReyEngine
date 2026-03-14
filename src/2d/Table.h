#pragma once
#include <format>
#include "ScrollArea.h"

namespace ReyEngine {
   class Table;
   class TableItem : public MetaDataInterface {
   public:
      std::string text;
      Pos<float> textPos;
      std::shared_ptr<Theme> overrideTheme;
      FontAlignment fontAlignment;
   };

   namespace Internal {
      class TableViewWidget : public Widget {
      public:
         TableViewWidget(Table& table)
         : _table(table)
         {}
         Rect<float> getCellRect(const Pos<size_t>& cellPos) const;
         Rect<float> getCellRect(const Pos<float>& mousePos) const;
         Pos<size_t> getCellRectCoords(const Pos<float>& mousePos) const;
         void render2D() const override;
         void _recalculate();
         float _columnWidth = 100;
         float _rowHeight = 25;
         std::vector<Line<float>> _vlines;
         std::vector<Line<float>> _hlines;
         size_t _columnCount = 0;
         size_t _rowCount = 0;
         bool _showVeritcalSeparators = true;
         bool _showHorizontalSeparators = true;
         std::map<size_t, std::map<size_t, std::unique_ptr<TableItem>>> _data;
         Table& _table;
      };
   }

   class Table : public ScrollArea {
   public:
      REYENGINE_OBJECT(Table)
      void _init() override {
         ScrollArea::_init();
         _view = make_child<Internal::TableViewWidget>(this, "tableView", *this);
      }
      template <typename T>
      TableItem* setData(const Pos<size_t>& cellPos, const std::string& text, const std::string& metaName, const T& data){
         auto ptr = setData(cellPos, text);
         ptr->setMetaData(metaName, data);
         return ptr;
      }
      TableItem* setData(const Pos<size_t>& cellPos, const std::string& text){
         auto [fst, snd] = _view->_data.try_emplace(cellPos.x).first->second.try_emplace(cellPos.y);
         auto& ptr = fst->second;
         if (!ptr){
            //add the child
            ptr = std::make_unique<TableItem>();
         }
         ptr->text = text;
         _view->_recalculate();
         return ptr.get();
      }
      template <typename T>
      std::optional<T*> getData(const Pos<size_t>& cellPos, const std::string& metaName) const {
         const auto foundX = _view->_data.find(cellPos.x);
         if (foundX == _view->_data.end()) return {};
         const auto foundY = foundX->second.find(cellPos.y);
         if (foundY == foundX->second.end()) return {};
         return foundY->second->getMetaData<T>(metaName);
      }

      void addWidgetAs(const std::shared_ptr<Widget>& w){

      }
      Handled _unhandled_input(const InputEvent&) override;
      void setRowCount(size_t rowCount) const {_view->_rowCount = rowCount; _view->_recalculate();}
      void setColumnCount(size_t columnCount) const {_view->_columnCount = columnCount; _view->_recalculate();}
      void setShowVerticalSeparators(bool show) const {_view->_showVeritcalSeparators = show;}
      void setShowHorizontalSeparators(bool show) const {_view->_showHorizontalSeparators = show;}
      void clear(){
         setRowCount(0);
         setColumnCount(0);
         _view->_data.clear();
         _view->_recalculate();
      }
      void setColumnWidth(float w){_view->_columnWidth = w;}
      void setRowHeight(float h){_view->_rowHeight = h;}
   protected:
      std::shared_ptr<Internal::TableViewWidget> _view;
      FontAlignment fontAlignment = {FontAlignmentHorizontal::LEFT, FontAlignmentVertical::CENTER};
      friend class Internal::TableViewWidget;
   };
}
