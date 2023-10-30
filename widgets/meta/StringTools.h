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
   std::string lrstrip(const std::string& s);
   int count(const std::string& s, char c);
   int lcount(const std::string& s, char c);
   int rcount(const std::string& s, char c);
   int countwhitel(const std::string& s);
   int countwhiter(const std::string& s);
   int countwhite(const std::string& s);
   std::string join(const std::string& s, const std::vector<std::string>& v);
   std::string listJoin(const std::vector<std::string>& v); //join into an array-like list
   std::vector<std::string> fromList(const std::string& s); //converts "{ARRAYS, OF, STRINGS} to vec<string>("ARRAYS", "OF", "STRINGS"). Strips leading and trailing whitespace on all elements.
}