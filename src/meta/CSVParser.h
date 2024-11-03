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
   CSVParser(const std::shared_ptr<ReyEngine::FileSystem::FileHandle>& file, bool hasHeader = false);

   /**
    * Returns a std::vector with all of the rows parsed from the file
    * @return : all rows from the file
    */
   const std::vector<Row>& getAllRows() const {return _data;}
   std::optional<std::reference_wrapper<Row>> getRow(size_t rowIndex) {
      if (rowIndex >= _data.size()) return std::nullopt;
      return std::ref(_data[rowIndex]);
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
   std::shared_ptr<ReyEngine::FileSystem::FileHandle> _file;
   std::vector<Row> _data;
   Row _header;

public:
   // Iterator class for rows of csv data
   class iterator : public std::iterator<std::forward_iterator_tag, std::string> {
   public:
      iterator(std::optional<std::reference_wrapper<CSVParser>> parser = std::nullopt)
      : _parser(parser)
      {}
      const Row& operator*() const {
         auto& r = _parser.value().get()._data.at(rowNo);
         return r;
      }
      iterator& operator++() {
         rowNo++;
         return *this;
      }

      bool operator!=(const iterator& other) const {
         if (!_parser) return false;
         if (!other._parser) {return rowNo < _parser.value().get().getAllRows().size();} //the end() case
         return _parser.value().get()._data[rowNo] != other._parser.value().get()._data[rowNo];
      }

      size_t getCurrentRowNo(){return rowNo;}
   private:
      size_t rowNo = 0;
      std::optional<std::reference_wrapper<CSVParser>> _parser;
   };

   iterator begin() {
      auto it = iterator(std::ref(*this));
      return it;
   }
   iterator end() const { return {};}
};