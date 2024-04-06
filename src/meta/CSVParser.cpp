#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CSVParser.h"

CSVParser::CSVParser(std::string filename) {
   _filename = filename;
   std::ifstream input(filename);
   if (!input.is_open()) {
      std::cerr << "Couldn't read file: " << filename << "\n";
   }

   std::vector<std::vector<std::string>> csvRows;

   for (std::string line; std::getline(input, line);) {
      std::istringstream ss(std::move(line));
      std::vector<std::string> row;
      if (!csvRows.empty()) {
         // We expect each row to be as big as the first row
         row.reserve(csvRows.front().size());
      }
      // std::getline can split on other characters, here we use ','
      for (std::string value; std::getline(ss, value, ',');) {
         row.push_back(std::move(value));
      }
      csvRows.push_back(std::move(row));
   }

   // Print out our table
   for (const std::vector<std::string>& row : csvRows) {
      for (const std::string& value : row) {
         std::cout << std::setw(10) << value;
      }
      std::cout << "\n";
   }
}