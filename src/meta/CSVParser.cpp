#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CSVParser.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
CSVParser::CSVParser(const ReyEngine::FileSystem::File& file, bool header)
: _file(file)
{
   auto it = _file.begin();
   auto firstLine = *it;
   if (firstLine.empty()) return; //empty
   
   auto split = string_tools::split(firstLine, CSV_SEP);
   auto expectedSize = split.size();
   if (header){
      _header = split;
      ++it; //move to next line
   }

   // We expect each row to be at least as big as the first row
   Row r;
   r.reserve(expectedSize);

   for (/**/; it!=_file.end(); ++it) {
      split = string_tools::split(*it, CSV_SEP);
      auto rowSize = split.size();
      if (rowSize < expectedSize) throw std::runtime_error("CSVParser: Not enough columns for row <" + to_string(it.getCurrentLineNo()) + "> in file " + _file.abs());
      r.insert(r.begin(), split.begin(), split.begin() + expectedSize);
      _data.push_back(r);
      r.clear();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> CSVParser::getHeaderIndex(const std::string &name) {
   if (!hasHeader()) return nullopt;
   size_t i = 0;
   for (const auto& cell : _header){
      if (name == cell){
         return i;
      }
      i++;
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
optional<string> CSVParser::getColumnName(int index){
   if(!hasHeader() || index >= _header.size()){
      return nullopt;
   }
   return _header[index];
}