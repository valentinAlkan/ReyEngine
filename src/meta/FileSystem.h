#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Property.h"
#include <fstream>
#include <iostream>

namespace ReyEngine::FileSystem {
   std::vector<char> readFile(const std::string& filePath);
   void writeFile(const std::string& filePath, const std::vector<char>&);

   using ComponentPath = std::string;
   static constexpr char COMPONENT_PATH_SEP = '/';
   static constexpr char _PATH_SEP_WIN = '\\';
   static constexpr char _PATH_SEP_OTHER = '/';
   static constexpr char FILESYSTEM_PATH_SEP =
      #ifdef _WIN32
         _PATH_SEP_WIN;
      #else
         _PATH_SEP_OTHER;
      #endif
   struct File;
   struct Directory;
   struct Path {
      Path() = default;
      Path(const std::string& path): paths(string_tools::pathSplit(path)){}
      Path(const std::vector<std::string>& paths): paths(paths){}
//      Path(const char* path): paths(string_tools::pathSplit(path)){}
      bool exists() const;
      Path head() const;
      std::optional<Path> tail() const;
      std::optional<File> toFile() const;
//      Path& join(const std::string&);
//      Path& join(const Path&);
      [[nodiscard]] std::string abs() const;
      const std::string str() const {return string_tools::join(FILESYSTEM_PATH_SEP, paths);}
      inline Path& operator+=(const std::vector<std::string>& rhs) {paths.insert(paths.end(), rhs.begin(), rhs.end()); return *this;}
      inline Path operator+(const std::vector<std::string>& rhs) const {Path newPath(*this); newPath += rhs; return newPath;}
      inline Path& operator+=(const Path& rhs) {*this += rhs.paths; return *this;}
      inline Path operator+(const Path& rhs) const {return *this + rhs.paths;}
      inline Path& operator+=(const std::string& rhs) {return *this += string_tools::pathSplit(rhs);}
      inline Path operator+(const std::string& rhs) const {return *this + string_tools::pathSplit(rhs);}
      explicit inline operator bool() const {return !paths.empty();}
      inline Path& operator=(const std::string& rhs){paths = string_tools::pathSplit(rhs); return *this;}
      inline bool operator==(const std::string& rhs) const {return paths == string_tools::pathSplit(rhs);}
      inline bool operator==(const Path& rhs) const {return paths == rhs.paths;}
      explicit inline operator std::string() const {return str();}

//      inline Path& operator=(const char* rhs){path = rhs; return *this;}
//      explicit inline operator const char*() {return path.c_str();}
//      inline Path& operator+=(const char* rhs) {path += std::string(rhs); return *this;}
      friend std::ostream& operator<<(std::ostream& os, const Path& _path) {os << _path.str(); return os;}
   protected:
      std::vector<std::string> paths;
   };

   struct File : public Path {
      File(){}
      File(const std::string& path): Path(path){}
      File(const char* path): Path(path){}
      File(const File& other){*this = other;}
      File& operator=(const File& other){(Path)*this = (Path&)other; return *this;}
      Directory dir();
      File(File&& other){
         (*this) = std::move(other);
      }
      File& operator=(File&& other){
         _ptr = other._ptr;
         _ifs = std::move(other._ifs);
         _open = other._open;
         return *this;
      }
      operator Directory() = delete;
      std::vector<char> readFile(){return FileSystem::readFile(str());}
      std::vector<char> readBytes(long long count);
      std::vector<char> readLine();
      void open();
      void close(){_ifs.close(); _open = false;}
      void seek(uint64_t i){ _ptr = i;}
      void save(){/*todo*/}
      bool eof(){return _ptr == _end;}
   private:
      bool _open=false;
      std::ifstream _ifs;
      std::fpos<std::mbstate_t> _ptr = 0;
      std::fpos<std::mbstate_t> _end;
   };

   struct Directory : public Path {
      Directory(){}
      Directory(const std::string& path): Path(path){}
      Directory(const char* path): Path(path){}
      operator File() = delete;
   };
}