#include "Table.h"
////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::setColumnCount(int numOfColumns){
   _columnCount = numOfColumns;
}

////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::addRow(std::vector<std::string> data) {
   auto lastRow = _data.end()--;
   int rowId = lastRow->first;
   std::map<int, std::string> tempMap;
   for(int i = 0; i < data.size(); i++){
      tempMap.insert({i, data.at(i)});
   }
   _data.insert({rowId, tempMap});
   _rowCount++;
}

////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::setColumnNames(std::vector<std::string> names) {
   std::map<int, std::string> tempMap;
   for(int i = 0; i < names.size(); i++){
      tempMap.insert({i, names.at(i)});
      setColumnCount(i + 1);
   }
   _data[0] = tempMap;
}

////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::setColumnTitle(int col, std::string title) {
   _data[0][col] = title;
}

////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::_init_() {
   _backPane = std::make_shared<Control>(std::to_string(Application::generateUniqueValue()) + "TableBackPane");
   _backPane->setAnchoring(BaseWidget::Anchor::FILL);
   addChild(_backPane);
   _tableGrid = std::make_shared<GridLayout>(std::to_string(Application::generateUniqueValue()) + "TableGridLayout");
}

////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Table::updateTable() {
   _tableGrid->
   for(int i = 0; i < _columnCount; i++){
   }
}