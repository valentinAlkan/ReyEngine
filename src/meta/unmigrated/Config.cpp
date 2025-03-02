#include "Config.h"
#include "lua.hpp"
#include <stdexcept>
using namespace std;
using namespace ReyEngine;

std::unique_ptr<Config> Config::_self;
/////////////////////////////////////////////////////////////////////////////////////////
Config::~Config() {}
/////////////////////////////////////////////////////////////////////////////////////////
Config &Config::instance() {
   if (!_self){
      _self.reset(new Config);
   }
   return *_self;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Config::loadConfig(const FileSystem::File& f) {
   instance()._handle = f.open();
   auto& L = instance()._LState;
   L = luaL_newstate();
   luaL_openlibs(L);
   auto error = luaL_dofile(L, f.str().c_str());
   if (error){
      throw std::runtime_error("Lua error loading file " + f.str());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Config::getString(const std::string &table, const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, table.c_str());
   lua_getfield(L, -1, element.c_str());
   auto retval = lua_tostring(L, -1);
   lua_remove(L, 2);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string Config::getString(const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, element.c_str());
   auto retval = lua_tostring(L, -1);
   lua_remove(L, 1);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
int Config::getInt(const std::string &table, const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, table.c_str());
   lua_getfield(L, -1, element.c_str());
   auto retval = lua_tointeger(L, -1);
   lua_remove(L, 2);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
int Config::getInt(const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, element.c_str());
   auto retval = lua_tointeger(L, -1);
   lua_remove(L, 1);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Config::getBool(const std::string &table, const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, table.c_str());
   lua_getfield(L, -1, element.c_str());
   auto retval = lua_toboolean(L, -1);
   lua_remove(L, 2);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Config::getBool(const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, element.c_str());
   auto retval = lua_toboolean(L, -1);
   lua_remove(L, 1);
   return retval;
}


/////////////////////////////////////////////////////////////////////////////////////////
double Config::getFloat(const std::string &table, const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, table.c_str());
   lua_getfield(L, -1, element.c_str());
   auto retval = lua_tonumber(L, -1);
   lua_remove(L, 2);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
double Config::getFloat(const std::string &element) {
   auto& L = instance()._LState;
   lua_getglobal(L, element.c_str());
   auto retval = lua_tonumber(L, -1);
   lua_remove(L, 1);
   return retval;
}