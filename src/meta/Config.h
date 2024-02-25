#pragma once
#include <memory>
#include "FileSystem.h"

class lua_State;
namespace ReyEngine {
   class Config {
   public:
      static void loadConfig(const FileSystem::File&);
      static std::string getString(const std::string& element);
      static std::string getString(const std::string& table, const std::string& element);
      static int getInt(const std::string& element);
      static int getInt(const std::string& table, const std::string& element);
      static double getFloat(const std::string& element);
      static double getFloat(const std::string& table, const std::string& element);
      static bool getBool(const std::string& element);
      static bool getBool(const std::string& table, const std::string& element);
      ~Config();
   private:
      Config(){};
      static Config& instance();
      static std::unique_ptr<Config> _self;
      lua_State* _LState = nullptr;
   };
}