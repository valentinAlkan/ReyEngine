#pragma once
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <functional>

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
      Path(const std::string& path): _path(path){}
      Path(const Path& other): _path(other._path){}
      bool exists() const {return std::filesystem::exists(_path);};
      Path head() const {return _path.filename().string();}
      std::optional<Path> tail() const {if (_path.has_parent_path())return _path.parent_path().string(); return std::nullopt;}
      std::optional<File> toFile() const;
      [[nodiscard]] std::string abs() const {return std::filesystem::absolute(_path).string();}
      [[nodiscard]] std::string str() const {return abs();}
      inline Path& operator+=(const Path& rhs) {_path /= rhs._path; return *this;}
      inline Path operator+(const Path& rhs) const {return (_path / rhs._path).string();}
//      inline Path& operator+=(const std::string& rhs) {return *this += string_tools::_pathplit(rhs);}
//      inline Path operator+(const std::string& rhs) const {return *this + string_tools::_pathplit(rhs);}
      explicit inline operator bool() const {return !_path.empty();}
      inline Path& operator=(const std::string& rhs){_path = rhs; return *this;}
      inline bool operator==(const std::string& rhs) const {return _path == rhs;}
      inline bool operator==(const Path& rhs) const {return _path == rhs._path;}
      explicit inline operator std::string() const {return str();}

//      inline Path& operator=(const char* rhs){path = rhs; return *this;}
//      explicit inline operator const char*() {return path.c_str();}
//      inline Path& operator+=(const char* rhs) {path += std::string(rhs); return *this;}
      friend std::ostream& operator<<(std::ostream& os, const Path& _path) {os << _path.str(); return os;}
   protected:
      std::filesystem::path _path;
   };

   struct File : public Path {
      File(){}
      File(const std::string& path): Path(path){}
      File(const char* path): Path(path){}
      File(const File& other): Path(other._path){}
      using Path::operator=;
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
      std::string readLine(); //read until we get to a new line (treats cr/nl as single newline).
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

   public:
      // Iterator class for lines of text
      class iterator : public std::iterator<std::forward_iterator_tag, std::string> {
      public:
         iterator(std::optional<std::reference_wrapper<FileSystem::File>> file) : _file(file){
            if (_file) {
               _file.value().get().open();
               operator++(); //load first line
            }
         }
         ~iterator(){
            if(_file) {
               _file.value().get().close();
            }
         }
         std::string operator*() const {return currentLine;}
         iterator& operator++() {
            currentLine = _file.value().get().readLine();
            lineNo++;
            return *this;
         }

         bool operator!=(const iterator& other) const {
            return currentLine != other.currentLine;
         }

         size_t getCurrentLineNo(){return lineNo;}
      private:
         size_t lineNo = 0;
         std::string currentLine;
         std::optional<std::reference_wrapper<FileSystem::File>> _file;
      };

      iterator begin() {
         auto it = iterator(std::ref(*this));
         return it;
      }
      iterator end() const { return {{}};}

   };

   struct Directory : public Path {
      Directory(){}
      Directory(const std::string& path): Path(path){}
      Directory(const char* path): Path(path){}
      operator File() = delete;
   };
}