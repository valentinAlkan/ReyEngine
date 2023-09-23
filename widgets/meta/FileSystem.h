#pragma once
#include <vector>
#include <string>

namespace FileSystem {
   std::vector<std::byte> loadFile(const std::string& filePath);
};
