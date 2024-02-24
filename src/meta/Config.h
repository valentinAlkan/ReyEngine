#pragma once
#include <memory>
#include "FileSystem.h"

namespace ReyEngine {
   class Config {
   public:
      static void loadConfig(FileSystem::File);
   private:
      static Config& instance();
      static std::unique_ptr<Config> _self;

   };
}