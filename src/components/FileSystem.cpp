#include "FileSystem.h"
#include "StringTools.h"
#include <iostream>
#include <cstring>
#include "Logger.h"

using namespace std;
using namespace ReyEngine;
using namespace FileSystem;

///////////////////////////////////////////////////////////////////////////////////////
Path::Path(const File &file): _path(file.str()){
   setType();
}

///////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<FileHandle> File::open() const {
   return std::shared_ptr<FileHandle>(new FileHandle(*this));
}

///////////////////////////////////////////////////////////////////////////////////////
void Path::setType() {
   try {
      if (!std::filesystem::exists(_path)) {
         pathType = EMPTY;
         return;
      }

      auto status = std::filesystem::status(_path);
      switch (status.type()) {
         case std::filesystem::file_type::regular:
            pathType = REGULAR_FILE;
            break;
         case std::filesystem::file_type::directory:
            pathType = DIRECTORY;
            break;
         case std::filesystem::file_type::symlink:
            pathType = SYMLINK;
            break;
         case std::filesystem::file_type::block:
            pathType = BLOCK_FILE;
            break;
         case std::filesystem::file_type::character:
            pathType = CHAR_FILE;
            break;
         case std::filesystem::file_type::fifo:
            pathType = FIFO;
            break;
         case std::filesystem::file_type::socket:
            pathType = SOCKET;
            break;
         default:
            pathType = OTHER;
            break;
      }
   } catch (const std::filesystem::filesystem_error& ex) {
      Logger::warn() << "Filesystem error setting type for " << _path << ": " << ex.what() << std::endl;
      pathType = OTHER;
   }
}
///////////////////////////////////////////////////////////////////////////////////////
File::File(const char *path): Path(path){}
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
void FileSystem::FileHandle::open() {
   _ifs = std::ifstream(_file.str(), std::ios::binary | std::ios::ate);
   if (!_ifs) {
      throw std::runtime_error(_file.abs() + ": " + std::strerror(errno));
   }
   _end = _ifs.tellg();
   _ptr = 0;
   _ifs.seekg(_ptr, std::ios::beg);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<char> FileSystem::FileHandle::readBytes(long long count) {
   std::vector<char> retval(count);
   readBytesInPlace(count, retval);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t FileSystem::FileHandle::readBytesInPlace(long long count, std::vector<char> &buffer) {
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
string FileSystem::FileHandle::readLine() {
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
//void FileSystem::writeFile(const std::string& filePath, const std::vector<char>&){
//   //todo: write file
//}

/////////////////////////////////////////////////////////////////////////////////////////
FileSystem::DirectoryContents::DirectoryContents(ReyEngine::FileSystem::Directory &) {

}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Path> FileSystem::DirectoryContents::files() const {
   return filter([](const Path& path) {
      return std::filesystem::is_regular_file(path._path);
   });
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Path> FileSystem::DirectoryContents::directories() const {
   return filter([](const Path& path) {
      return std::filesystem::is_directory(path._path);
   });
}

/////////////////////////////////////////////////////////////////////////////////////////
std::set<Path> FileSystem::Directory::listContents() {
   std::set<Path> contents;
   if (!exists()) {
      throw std::runtime_error("Directory does not exist: " + str());
   }

   if (!std::filesystem::is_directory(_path)) {
      throw std::runtime_error("Path is not a directory: " + str());
   }

   std::error_code ec;
   for (const auto& entry : std::filesystem::directory_iterator(_path, ec)) {
      if (ec) {
         throw std::runtime_error("Error reading directory: " + ec.message());
      }
      contents.emplace(entry.path().string());
   }

   return contents;
}