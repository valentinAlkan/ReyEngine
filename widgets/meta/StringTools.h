#pragma once

#include <vector>
#include <string>
#include <functional>

namespace string_tools{
   std::vector<std::string> split(const std::string& s, const std::string& delimString);
   std::string lstrip(const std::string& s, char c);
   std::string lstrip(const std::string& s);
   std::string rstrip(const std::string& s, char c);
   std::string rstrip(const std::string& s);
   std::string strip(const std::string& s, const std::function<bool(char c)>& charMatch, bool rtol = false);
}