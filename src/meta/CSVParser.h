#pragma once
#include <string>
#include <iostream>
#include <optional>
#include <vector>
#include "FileSystem.h"

class CSVParser {
public:
   CSVParser(const ReyEngine::FileSystem::File& file, bool hasHeader = false);

   /**
    * Returns a std::vector with all of the rows parsed from the file
    * @return : all rows from the file
    */
   std::vector<std::vector<std::string>> getAllRows();

   /**
    * Returns the next row and increments the index
    * @return : the next row in the file; nullopt if the end of file exists
    */
   std::optional<std::vector<std::string>> getNextRow();

   /**
    * Returns the header row of the file
    * @return : the header row of the file; nullopt if the file has no header row
    */
   std::optional<std::vector<std::string>> getHeader();

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
   std::optional<int> getHeaderIndex(std::string name);

   bool hasNext();

private:
   ReyEngine::FileSystem::File _file;
   std::vector<std::vector<std::string>> _csvRows;
   int _index;
   bool _hasHeader;
   std::vector<std::string> _header;
};