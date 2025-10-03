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
   parsePath();
}

///////////////////////////////////////////////////////////////////////////////////////
File Path::toFile() const {return *this;}
Directory Path::toDir() const {return *this;}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
void Path::erase(EraseType eraseType) {
   if (eraseType == MUST_EXIST && !exists()){
      throw std::runtime_error("Failed to erase file " + str() + ": File did not exist and was required to");
   }

   try {
      std::error_code ec;

      switch (_pathType) {
         case REGULAR_FILE:
         case SYMLINK:
         case BLOCK_FILE:
         case CHAR_FILE:
         case FIFO:
         case SOCKET:
            if (!std::filesystem::remove(_path, ec)) {
               if (ec) {
                  throw std::runtime_error("Failed to erase file " + str() + ": " + ec.message());
               }
            }
            break;

         case DIRECTORY:{
            auto removedCount = std::filesystem::remove_all(_path, ec);
            if (ec) {
               throw std::runtime_error("Failed to erase directory " + str() + ": " + ec.message());
            }
            Logger::debug() << "Removed " << removedCount << " items from directory " << _path << std::endl;
            break;}

         case EMPTY:
            // Path doesn't exist, nothing to do
            Logger::warn() << "Path does not exist: " << _path << std::endl;
            return;

         case OTHER:
         default:
            // Try to remove anyway, let filesystem decide
            if (!std::filesystem::remove(_path, ec)) {
               if (ec) {
                  throw std::runtime_error("Failed to erase path " + str() + ": " + ec.message());
               }
            }
            break;
      }

      _pathType = EMPTY;

   } catch (const std::filesystem::filesystem_error& ex) {
      throw std::runtime_error("Filesystem error erasing " + str() + ": " + ex.what());
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void Path::create(bool createParent) {
   if (exists()){
      // just in case the type is different, do not let us create files if there is already
      // an object at the location
      throw std::runtime_error("Cannot create: path exists");
   }

   try {
      // Check if parent directory exists, create it if it doesn't
      if (createParent) {
         auto parentPath = _path.parent_path();
         if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::error_code ec;
            if (!std::filesystem::create_directories(parentPath, ec) && ec) {
               throw std::runtime_error("Failed to create parent directories for " + str() + ": " + ec.message());
            }
         }
      }

      switch (_pathType){
         case DIRECTORY:{
         // Create directory
         std::error_code ec;
         if (!std::filesystem::create_directories(_path, ec)) {
            if (ec) {
               throw std::runtime_error("Failed to create directory " + str() + ": " + ec.message());
            }
         }
        break;}
      case REGULAR_FILE:{
         std::ofstream file(_path);
         if (!file) throw std::runtime_error("Failed to create file " + str() + ": " + std::strerror(errno));
         file.close();}
      default:
         throw std::runtime_error("Creation not supported for this path type!");
      }

   } catch (const std::filesystem::filesystem_error& ex) {
      throw std::runtime_error("Filesystem error creating " + str() + ": " + ex.what());
   }
}

///////////////////////////////////////////////////////////////////////////////////////
bool Path::createIfNotExist(bool createParent) {
   if (!exists()){
      create(createParent);
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////////
void Path::overwrite(bool createParent) {
   auto pathType = _pathType;
   erase(CAN_EXIST);
   _pathType = pathType;
   create(createParent);
}

///////////////////////////////////////////////////////////////////////////////////////
void Path::parsePath() {
   //expand tilde's by exploding path and examining each element
   using PathParsePair = std::pair<bool, std::vector<std::filesystem::path>>;
   auto components = PathParsePair(false, {_path.begin(), _path.end()});
   auto& doneParsingPath = components.first;
   auto strip_tilde = [](PathParsePair& pair){
      auto& components = pair.second;
      for (size_t i = 0; i < components.size(); i++) {
         std::string comp_str = components.at(i).string();
         if (comp_str == "~" || comp_str.starts_with("~")) {
            components = {components.begin()+i, components.end()};
            components.front() = CrossPlatform::getUserDir();
            return;
         }
      }
      //done
      pair.first = true;
   };
   while (!doneParsingPath){
      strip_tilde(components);
   }

   //rebuild path
   _path.clear();
   for (const auto& component : components.second){
      _path /= component;
   }

   try {
      if (!std::filesystem::exists(_path)) {
         _pathType = EMPTY;
         return;
      }

      auto status = std::filesystem::status(_path);
      switch (status.type()) {
         case std::filesystem::file_type::regular:
            _pathType = REGULAR_FILE;
            break;
         case std::filesystem::file_type::directory:
            _pathType = DIRECTORY;
            break;
         case std::filesystem::file_type::symlink:
            _pathType = SYMLINK;
            break;
         case std::filesystem::file_type::block:
            _pathType = BLOCK_FILE;
            break;
         case std::filesystem::file_type::character:
            _pathType = CHAR_FILE;
            break;
         case std::filesystem::file_type::fifo:
            _pathType = FIFO;
            break;
         case std::filesystem::file_type::socket:
            _pathType = SOCKET;
            break;
         default:
            _pathType = OTHER;
            break;
      }
   } catch (const std::filesystem::filesystem_error& ex) {
      Logger::warn() << "Filesystem error setting type for " << _path << ": " << ex.what() << std::endl;
      _pathType = OTHER;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Directory> FileSystem::Path::getParentDirectory() const {
   auto parent = Directory(head());
   if (parent.exists()){
      return parent;
   }
   return {};
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<FileHandle> File::open() const {
   return std::shared_ptr<FileHandle>(new FileHandle(*this));
}

///////////////////////////////////////////////////////////////////////////////////////
File File::changeExtension(const std::string &newExtension) const {
   auto dir = getParentDirectory().value();
   auto split = string_tools::split(tail(), '.');
   auto swapped = string_tools::join("", vector<string>(split.begin(), split.end()-1)) + newExtension;
   return dir / swapped;
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

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::set<Path> FileSystem::Directory::listContents() const {
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