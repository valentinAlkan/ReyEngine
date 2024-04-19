#include "FileSystem.h"
#include <iostream>
#include <cstring>
#include <filesystem>
#include "Application.h"
#include "StringTools.h"

using namespace std;
using namespace ReyEngine::FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<char> ReyEngine::FileSystem::readFile(const std::string &filePath) {
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

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::FileSystem::File::open() {
   if (!_open) {
      _ifs = std::ifstream(_path, std::ios::binary | std::ios::ate);
      if (!_ifs) {
         throw std::runtime_error(abs() + ": " + std::strerror(errno));
      }
      _end = _ifs.tellg();
   }
   _ptr = 0;
   _ifs.seekg(_ptr, std::ios::beg);
   _open = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<char> ReyEngine::FileSystem::File::readBytes(long long count) {
   if (!_open) throw std::runtime_error("File " + abs() + " is not open!");
   auto size = std::size_t(_end - _ptr);
   if (size == 0) return {};
   if (count > size) count = size;
   std::vector<char> buffer(count);
   _ifs.seekg(_ptr, std::ios::beg);
   if (!_ifs.read((char *) buffer.data(), buffer.size()))
      throw std::runtime_error(str() + ": " + std::strerror(errno));
   _ptr += count;
   return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////
string ReyEngine::FileSystem::File::readLine() {
   if (!_open) throw std::runtime_error("File " + abs() + " is not open!");
   if (_end - _ptr == 0) return{}; //empty file
   bool carriageReturn = false;
   string retval;
   char c;
   while (std::size_t(_end - _ptr)){
      if (!_ifs.read((char *) &c, 1)) {
         throw std::runtime_error(str() + ": " + std::strerror(errno));
      }
      _ptr += 1;
      if (c == '\n') {
         break;
      }
      if (c == '\r') {
         //steal the carriage return for now
         carriageReturn = true;
         continue;
      } else if (carriageReturn) {
         //add back in the carriage return we stole since it didn't precede a lf char
         c = '\r';
         carriageReturn = false;
      }
      retval += c;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::FileSystem::writeFile(const std::string& filePath, const std::vector<char>&){
   //todo: write file
}