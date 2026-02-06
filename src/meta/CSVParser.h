#pragma once
#include <string>
#include <iostream>
#include <optional>
#include <vector>
#include "FileSystem.h"

class CSVParser {
public:
   using Row = std::vector<std::string>;
   CSVParser(const std::shared_ptr<ReyEngine::FileSystem::FileHandle>& file, bool hasHeader, char csv_sep = ',');
   CSVParser(const std::string& filePath, bool hasHeader = false, char csv_sep = ',');
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
   std::optional<const Row*> getHeader() const {return &_header;}
   bool hasHeader(){return !_header.empty();}

private:
   std::shared_ptr<ReyEngine::FileSystem::FileHandle> _file;
   std::vector<Row> _data;
   Row _header;

public:
   // Iterator class for rows of csv data
   template <typename T>
   class iterator_impl {
   public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = T;
      using pointer           = T*;
      using reference         = T&;

      iterator_impl(std::vector<Row>* data_ptr, size_t index): _data_ptr(data_ptr), _index(index) {}
      reference operator*() const { return (*_data_ptr)[_index]; }
      pointer operator->() const { return &((*_data_ptr)[_index]); }
      iterator_impl& operator++() {
         _index++;
         return *this;
      }

      iterator_impl operator++(int) {
         iterator_impl tmp = *this;
         ++(*this);
         return tmp;
      }
      friend bool operator==(const iterator_impl& a, const iterator_impl& b) {return a._data_ptr == b._data_ptr && a._index == b._index;}
      friend bool operator!=(const iterator_impl& a, const iterator_impl& b) {return !(a == b);}
   private:
      std::vector<Row>* _data_ptr;
      size_t _index;
   };
};