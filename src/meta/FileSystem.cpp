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
   
   _ifs = std::ifstream(abs(), std::ios::binary | std::ios::ate);
   if (!_ifs){
      throw std::runtime_error(abs() + ": " + std::strerror(errno));
   }
   _end = _ifs.tellg();
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
   auto size = std::size_t(_end - _ptr);
   if (size == 0) return {};
   bool carriageReturn = false;
   string retval;
   char c;
   bool done = false;
   while (!done){
      _ifs.seekg(_ptr, std::ios::beg);
      if (!_ifs.read((char *) &c, 1))
         throw std::runtime_error(str() + ": " + std::strerror(errno));
      _ptr += 1;
      if (c == '\n'){
         done = true;
         continue;
      }
      if (c == '\r'){
         //steal the carriage return for now
         carriageReturn = true;
         continue;
      } else if (carriageReturn){
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


/////////////////////////////////////////////////////////////////////////////////////////
bool ReyEngine::FileSystem::Path::exists() const {
   return std::filesystem::exists(abs());
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::FileSystem::Path ReyEngine::FileSystem::Path::head() const {
   auto retval = vector<string>(paths.begin(), paths.end()-1);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<ReyEngine::FileSystem::Path> ReyEngine::FileSystem::Path::tail() const {
   //todo:
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string ReyEngine::FileSystem::Path::abs() const {
   Path retval = CrossPlatform::getExePath();
   auto fullPath = retval.head() + paths;
   return string_tools::pathJoin(fullPath.paths);
}