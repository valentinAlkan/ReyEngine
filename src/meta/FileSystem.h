#pragma once
#include <vector>
#include <string>
#include <optional>

namespace ReyEngine::FileSystem {
   std::vector<char> loadFile(const std::string& filePath);

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
   protected:
      std::string path;
   };

   struct File : public Path {
      File(){}
      File(const std::string& path): Path(path){}
      File(const char* path): Path(path){}
      operator Directory() = delete;
   };

   struct Directory : public Path {
      Directory(){}
      Directory(const std::string& path): Path(path){}
      Directory(const char* path): Path(path){}
      operator File() = delete;
   };

}