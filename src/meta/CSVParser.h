#pragma once
#include <string>
#include <iostream>
#include <optional>
#include <vector>
using namespace std;

class CSVParser {
public:
   CSVParser(string filename, bool hasHeader = false);

   /**
    * Returns a vector with all of the rows parsed from the file
    * @return : all rows from the file
    */
   vector<vector<string>> getAllRows();

   /**
    * Returns the next row and increments the index
    * @return : the next row in the file; nullopt if the end of file exists
    */
   optional<vector<string>> getNextRow();

   /**
    * Returns the header row of the file
    * @return : the header row of the file; nullopt if the file has no header row
    */
   optional<vector<string>> getHeader();

   /**
    * Gets the header value of the given column
    * @param index : the column number of interest
    * @return : the header value of the column; nullopt if the index is greater then the number of columns, or the file has no header
    */
   optional<string> getColumnName(int index);

   /**
    * gets the column number of the given header
    * @param name : the header string of interest
    * @return : the index of the given header string; nullopt if the header doesn't exist, or the file has no header
    */
   optional<int> getHeaderIndex(string name);

   bool hasNext();

private:
   string _filename;
   vector<vector<string>> _csvRows;
   int _index;
   bool _hasHeader;
   vector<string> _header;
};