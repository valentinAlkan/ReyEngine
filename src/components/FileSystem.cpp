#include "FileSystem.h"
#include "StringTools.h"
#include <iostream>
#include <cstring>

using namespace std;
using namespace ReyEngine::FileSystem;

///////////////////////////////////////////////////////////////////////////////////////
Path::Path(const File &file): _path(file.str()){}
///////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<FileHandle> File::open() const {
   return std::shared_ptr<FileHandle>(new FileHandle(*this));
}

///////////////////////////////////////////////////////////////////////////////////////
File::File(const char *path): Path(path){}
File::File(const File& other): Path(other){}
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
   std::vector<char> retval(count);
   readBytesInPlace(count, retval);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t ReyEngine::FileSystem::FileHandle::readBytesInPlace(long long count, std::vector<char> &buffer) {
   if (count < 0) {throw std::invalid_argument("Count cannot be negative");}
   auto remaining = _end - _ptr;
   if (remaining == 0) return 0;
   auto bytesToRead = std::min<long>(count, remaining);
   if (buffer.size() < bytesToRead) {
      buffer.resize(bytesToRead);
   }
   if (!_ifs.seekg(_ptr, std::ios::beg)) {
      throw std::runtime_error(_file.str() + ": Seek operation failed");
   }
   if (!_ifs.read(buffer.data(), bytesToRead) || _ifs.gcount() != static_cast<std::streamsize>(bytesToRead)) {
      throw std::runtime_error(_file.str() + ": Read operation failed");
   }

   _ptr += bytesToRead;
   return bytesToRead;
}


/////////////////////////////////////////////////////////////////////////////////////////
string ReyEngine::FileSystem::FileHandle::readLine() {
   if (_end - _ptr == 0) return {};  // empty file
   string retval;
   retval.reserve(128);  // Reserve reasonable initial capacity
   bool carriageReturn = false;
   char c;
   while (std::size_t(_end - _ptr) > 0) {
      if (!_ifs.get(c)) {
         if (_ifs.eof()) break;
         throw std::runtime_error(_file.str() + ": Read failed");
      }
      _ptr += 1;
      if (c == '\n') {
         break;
      }
      if (c == '\r') {
         // Peek at next character without consuming it
         if (_ptr < _end && _ifs.peek() == '\n') {
            carriageReturn = false;
            continue;  // Skip '\r' when it's part of '\r\n'
         }
         // Single '\r' should be preserved
         retval += c;
      } else {
         retval += c;
      }
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
//void ReyEngine::FileSystem::writeFile(const std::string& filePath, const std::vector<char>&){
//   //todo: write file
//}