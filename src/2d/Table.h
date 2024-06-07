#pragma once
#include "TileMap.h"
#include "Control.h"
#include "Layout.h"

namespace ReyEngine {
   class Table : public TileMap {
   public:
      /**
       * sets the column count
       * @param numOfColumns : Number of columns desired
       */
      void setColumnCount(int numOfColumns);

      /**
       * adds a row of data to the table
       * @param data : vector of the data in strings
       */
      void addRow(std::vector<std::string> data);

      /**
       * sets the titles of the columns
       * @param names : the titles of the columns
       */
      void setColumnNames(std::vector<std::string> names);

      /**
       * sets the specific column to the title
       * @param col : the index of the column
       * @param title : the title to change to
       */
      void setColumnTitle(int col, std::string title);
   private:
      std::map<int, std::string> _columnTitles;

      // first index is the row number, second is the column number
      std::map<int, std::map<int, std::string>> _data;

   };
}
