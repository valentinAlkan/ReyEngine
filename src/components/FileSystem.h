#pragma once
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <functional>
#include <set>
#include <ranges>
#include <algorithm>

namespace ReyEngine::FileSystem {
   static constexpr char SCENE_PATH_SEP = '/';
   static constexpr char _PATH_SEP_WIN = '\\';
   static constexpr char _PATH_SEP_OTHER = '/';
   static constexpr char FILESYSTEM_PATH_SEP =
      #ifdef _WIN32
         _PATH_SEP_WIN;
      #else
         _PATH_SEP_OTHER;
      #endif
   struct FileHandle;
   struct Directory;
   struct File;
   struct DirectoryContents;
   struct Path {
      enum PathType {EMPTY, REGULAR_FILE, DIRECTORY, SYMLINK, BLOCK_FILE, CHAR_FILE, FIFO, SOCKET, OTHER};
//      Path();
      Path(const File& file);
      Path(const char* path): _path(path){setType();}
      Path(const std::string& path): _path(path){setType();}
      [[nodiscard]] bool exists() const {return std::filesystem::exists(_path);};
      [[nodiscard]] Path head() const {return _path.filename().string();}
      [[nodiscard]] std::optional<Path> tail() const {if (_path.has_parent_path())return _path.parent_path().string(); return std::nullopt;}
      [[nodiscard]] std::optional<FileHandle> toFile() const;
      [[nodiscard]] std::string abs() const {return std::filesystem::absolute(_path).string();}
      [[nodiscard]] std::string str() const {return abs();}
      PathType pathType;
      inline Path& operator+=(const Path& rhs) {_path /= rhs._path; return *this;}
      inline Path operator+(const Path& rhs) const {return (_path / rhs._path).string();}
      inline Path operator+(const char* rhs) const {return {*this + std::string(rhs)};}
      inline Path operator+(const std::string& rhs) const {return {_path.string() + rhs};}
//      inline Path& operator+=(const std::string& rhs) {return *this += string_tools::_pathplit(rhs);}
//      inline Path operator+(const std::string& rhs) const {return *this + string_tools::_pathplit(rhs);}
      explicit inline operator bool() const {return !_path.empty();}
      inline Path& operator=(const std::string& rhs){_path = rhs; return *this;}
      inline bool operator==(const std::string& rhs) const {return _path == rhs;}
      inline bool operator==(const Path& rhs) const {return _path == rhs._path;}
      [[nodiscard]] inline operator std::string() const {return str();}

      inline bool operator<(const Path& rhs) const {return _path < rhs._path;}
      inline bool operator>(const Path& rhs) const {return _path > rhs._path;}
      inline bool operator<=(const Path& rhs) const {return _path <= rhs._path;}
      inline bool operator>=(const Path& rhs) const {return _path >= rhs._path;}
      inline bool operator!=(const Path& rhs) const {return _path != rhs._path;}

//      inline Path& operator=(const char* rhs){path = rhs; return *this;}
//      explicit inline operator const char*() {return path.c_str();}
//      inline Path& operator+=(const char* rhs) {path += std::string(rhs); return *this;}
      inline auto operator<=>(const Path& rhs) const {return _path <=> rhs._path;}
      friend std::ostream& operator<<(std::ostream& os, const Path& _path) {os << _path.str(); return os;}
   protected:
      void setType();
      std::filesystem::path _path;
      friend struct DirectoryContents;
   };

   struct File : public Path {
      using Path::operator=;
      // A path to a disk. Cannot read from.
//      File(){}
      File(const std::string& path): Path(path){}
      File(std::string_view path): File(std::string(path)){}
      File(const char* path);
      File(const File& other) = default;
      File& operator=(const File& other) = default;
      void clear(){_path.clear();}
      Directory dir();
      operator Directory() = delete;
      [[nodiscard]] std::shared_ptr<FileHandle> open() const;
   };

   struct FileHandle {
      // A handle to a file that has been opened and is available to read from.
      ~FileHandle(){close();}
      std::vector<char> readFile();
      std::vector<char> readBytes(long long count);
      size_t readBytesInPlace(long long count, std::vector<char>& buffer);
      std::string readLine(); //read until we get to a new line (treats cr/nl as single newline).
      std::optional<char> peek() const;
      void seek(uint64_t i){ _ptr = i;}
      void save(){throw std::runtime_error("not implemented");/*todo*/}
      bool isEof() const {return _ptr == _end;}
      File file(){return _file;}
   protected:
      FileHandle(const File& file): _file(file){open();}
   private:
      void open();
      void close(){_ifs.close();}
      std::ifstream _ifs;
      std::fpos<std::mbstate_t> _ptr = 0;
      std::fpos<std::mbstate_t> _end;
      File _file;

   public:
      // Iterator class for lines of text
      class iterator {
      public:
         // Iterator traits
         using iterator_category = std::forward_iterator_tag;
         using value_type = std::string;
         using difference_type = std::ptrdiff_t;
         using pointer = std::string*;
         using reference = std::string&;
         iterator(std::optional<std::reference_wrapper<FileSystem::FileHandle>> file) : _file(file){
            if (_file) {
               _file.value().get().open();
               operator++(); //load first line
            }
         }
         ~iterator(){}
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
         std::optional<std::reference_wrapper<FileSystem::FileHandle>> _file;
      };
      iterator begin() {
         auto it = iterator(std::ref(*this));
         return it;
      }
      iterator end() const { return {{}};}
      friend class File;
   };

   struct DirectoryContents{
      DirectoryContents(Directory& dir);
      [[nodiscard]] std::vector<Path> sort(std::function<std::vector<Path>(std::set<Path>&)> sortFunc){return sortFunc(_contents);}
      [[nodiscard]] auto begin() const { return _contents.begin(); }
      [[nodiscard]] auto end() const { return _contents.end(); }
      [[nodiscard]] size_t size() const { return _contents.size(); }
      [[nodiscard]] bool empty() const { return _contents.empty(); }
      [[nodiscard]] bool contains(const Path& path) const { return _contents.contains(path); }

      template<typename Predicate>
      [[nodiscard]] std::vector<Path> filter(Predicate pred) const{
         std::vector<Path> result;
         auto filtered = _contents | std::views::filter(pred);
         std::ranges::copy(filtered, std::back_inserter(result));
         return result;
      }

      [[nodiscard]] std::vector<Path> files() const;
      [[nodiscard]] std::vector<Path> directories() const;
   private:
      std::set<Path> _contents;
   };

   struct Directory : public Path {
//      Directory(){}
      Directory(const std::string& path): Path(path){}
      Directory(const char* path): Path(path){}
      Directory(Directory& other) = default;
      Directory& operator=(const Directory& other){_path = other._path; return *this;}
      operator FileHandle() = delete;
      std::set<Path> listContents();
   };
}