#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CSVParser.h"

CSVParser::CSVParser(string filename, bool hasHeader) {
   _filename = filename;
   _hasHeader = hasHeader;
   ifstream input(filename);
   _index = 1;
   if (!input.is_open()) {
      cerr << "Couldn't read file: " << filename << "\n";
   }

   for (string line; getline(input, line);) {
      istringstream ss(move(line));
      vector<string> row;
      if (!_csvRows.empty()) {
         // We expect each row to be as big as the first row
         row.reserve(_csvRows.front().size());
      }
      // std::getline can split on other characters, here we use ','
      for (string value; getline(ss, value, ',');) {
         row.push_back(move(value));
      }
      _csvRows.push_back(move(row));
   }
}

vector<vector<string>> CSVParser::getAllRows() {
   return _csvRows;
}

optional<vector<string>> CSVParser::getNextRow() {
   if(_index >= _csvRows.size()){
      return nullopt;
   }
   return _csvRows[_index++];
}

optional<vector<string>> CSVParser::getHeader() {
   if(!_hasHeader){
      return nullopt;
   }
   return _csvRows[0];
}