#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <cstring>

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<char> FileSystem::loadFile(const std::string &filePath) {
   std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
   if (!ifs)
      throw std::runtime_error(filePath + ": " + std::strerror(errno));

   auto end = ifs.tellg();
   ifs.seekg(0, std::ios::beg);

   auto size = std::size_t(end - ifs.tellg());

   if (size == 0) return {};

   std::vector<char> buffer(size);

   if (!ifs.read((char *) buffer.data(), buffer.size()))
      throw std::runtime_error(filePath + ": " + std::strerror(errno));
   return buffer;
}