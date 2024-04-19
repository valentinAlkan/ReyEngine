#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CSVParser.h"

using namespace std;
using namespace ReyEngine;

CSVParser::CSVParser(const ReyEngine::FileSystem::File& file, bool hasHeader)
: _file(file)
, _hasHeader(hasHeader)
{
   bool gotHeader = false;
   _file.open();

   for (string line; line.empty(); line = _file.readLine()) {
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
      if(hasHeader && !gotHeader){
         _header = row;
         gotHeader = true;
      } else {
         _csvRows.push_back(move(row));
      }
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
   return _header;
}

optional<string> CSVParser::getColumnName(int index){
   if(!_hasHeader || index >= _header.size()){
      return nullopt;
   }
   return _header[index];
}

optional<int> CSVParser::getHeaderIndex(string name) {
   if(!_hasHeader){
      return nullopt;
   }
   for(int i = 0; i < _header.size(); i++){
      if(_header[i].compare(name) == 0){
         return i;
      }
   }
   return nullopt;
}

bool CSVParser::hasNext() {
   if (_index >= _csvRows.size()){
      return false;
   }
   return true;
}