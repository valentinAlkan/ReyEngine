#pragma once
#include <string>
#include <iostream>


class CSVParser {
public:
   CSVParser(std::string filename);

private:
   std::string _filename;
};