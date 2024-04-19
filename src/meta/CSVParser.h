#pragma once
#include <string>
#include <iostream>
#include <optional>
#include <vector>
#include "FileSystem.h"

class CSVParser {
public:
   using Row = std::vector<std::string>;
   static constexpr char CSV_SEP = ',';
   CSVParser(const ReyEngine::FileSystem::File& file, bool hasHeader = false);

   /**
    * Returns a std::vector with all of the rows parsed from the file
    * @return : all rows from the file
    */
   const std::vector<Row>& getAllRows(){return _data;}
   const std::optional<std::reference_wrapper<Row>> getRow(size_t rowIndex) {
      if (rowIndex >= _data.size()) return std::nullopt;
      return _data[rowIndex];
   }

   /**
    * Gets the header value of the given column
    * @param index : the column number of interest
    * @return : the header value of the column; nullopt if the index is greater then the number of columns, or the file has no header
    */
   std::optional<std::string> getColumnName(int index);

   /**
    * gets the column number of the given header
    * @param name : the header std::string of interest
    * @return : the index of the given header std::string; nullopt if the header doesn't exist, or the file has no header
    */
   std::optional<size_t> getHeaderIndex(const std::string& name);
   std::optional<const std::reference_wrapper<Row>> getHeader(){return _header;}
   bool hasHeader(){return !_header.empty();}

private:
   ReyEngine::FileSystem::File _file;
   std::vector<Row> _data;
   Row _header;
};