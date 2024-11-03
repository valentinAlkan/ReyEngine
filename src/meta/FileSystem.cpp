#include "FileSystem.h"
#include <iostream>
#include <cstring>
#include "StringTools.h"

using namespace std;
using namespace ReyEngine::FileSystem;

///////////////////////////////////////////////////////////////////////////////////////
Path::Path(const File &file): _path(file.str()){}
///////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<FileHandle> File::open() const {
   return std::shared_ptr<FileHandle>(new FileHandle(*this));
}

///////////////////////////////////////////////////////////////////////////////////////
std::vector<char> FileHandle::readFile(){
   auto end = _ifs.tellg();
   _ifs.seekg(0, std::ios::beg);
   auto size = std::size_t(end - _ifs.tellg());
   if (size == 0) return {};

   std::vector<char> buffer(size);

   if (!_ifs.read((char *) buffer.data(), buffer.size()))
      throw std::runtime_error(_file.str() + ": " + std::strerror(errno));
   return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::FileSystem::FileHandle::open() {
   _ifs = std::ifstream(_file.str(), std::ios::binary | std::ios::ate);
   if (!_ifs) {
      throw std::runtime_error(_file.abs() + ": " + std::strerror(errno));
   }
   _end = _ifs.tellg();
   _ptr = 0;
   _ifs.seekg(_ptr, std::ios::beg);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<char> ReyEngine::FileSystem::FileHandle::readBytes(long long count) {
   auto size = std::size_t(_end - _ptr);
   if (size == 0) return {};
   if (count > size) count = size;
   std::vector<char> buffer(count);
   _ifs.seekg(_ptr, std::ios::beg);
   if (!_ifs.read((char *) buffer.data(), buffer.size()))
      throw std::runtime_error(_file.str() + ": " + std::strerror(errno));
   _ptr += count;
   return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////
string ReyEngine::FileSystem::FileHandle::readLine() {
   if (_end - _ptr == 0) return{}; //empty file
   bool carriageReturn = false;
   string retval;
   char c;
   while (std::size_t(_end - _ptr)){
      if (!_ifs.read((char *) &c, 1)) {
         throw std::runtime_error(_file.str() + ": " + std::strerror(errno));
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
//void ReyEngine::FileSystem::writeFile(const std::string& filePath, const std::vector<char>&){
//   //todo: write file
//}