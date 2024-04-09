#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Property.h"
#include <fstream>

namespace ReyEngine::FileSystem {
   std::vector<char> readFile(const std::string& filePath);
   void writeFile(const std::string& filePath, const std::vector<char>&);

   using ComponentPath = std::string;
   static constexpr char COMPONENT_PATH_SEP = '/';
   struct File;
   struct Directory;
   struct Path {
      Path(){};
      Path(const std::string& path): path(path){}
      Path(const char* path): path(path){}
      bool exists() const;
      std::optional<Path> head() const;
      std::optional<Path> tail() const;
      std::optional<File> toFile() const;
      std::optional<Directory> toDirectory() const;
      std::string abs() const;
      const std::string& str() const {return path;}
      inline std::string operator+(const Path& rhs) const {return path + rhs.str();}
      inline std::string operator+(const char* rhs) const {return path + std::string(rhs);}
      inline operator std::string() const {return path;}
      inline operator const char*() {return path.c_str();}
      inline Path& operator=(const char* rhs){path = rhs; return *this;}
      inline Path& operator=(const std::string& rhs){path = rhs; return *this;}
      inline operator bool() const {return !path.empty();}
      std::ostream& operator<<(std::ostream& os) const {os << path; return os;}
   protected:
      std::string path;
   };

   struct File : public Path {
      File(){}
      File(const std::string& path): Path(path){}
      File(const char* path): Path(path){}
      File(File& other) = delete;
      File& operator=(File& other) = delete;
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
      using Path::operator<<;
      std::vector<char> readFile(){return FileSystem::readFile(path);}
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
      using Path::operator<<;
   };
}