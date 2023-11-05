#include "StringTools.h"

using namespace string_tools;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::split(const std::string &s, const std::string &delimString) {
   vector<string> retval;
   size_t start = 0;
   string substr = s;
   size_t found;
   auto find = [&](){found = substr.find(delimString); return found;};
   while(find() != string::npos){
      retval.emplace_back(s, start, found);
      start += found + delimString.size();
      if (start >= s.size()){
         break;
      }
      substr = string(s, start, s.size() - start);
   }
   retval.push_back(substr);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::lstrip(const std::string &s, char c) {
   auto charMatch = [c](char _c){
      return _c == c;
   };
   return strip(s, charMatch);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::rstrip(const std::string &s, char c) {
   auto charMatch = [c](char _c){
      return _c == c;
   };
   return strip(s, charMatch, true);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::lstrip(const std::string &s) {
   auto charMatch = [](char _c){
      return ::isspace(_c);
   };
   return strip(s, charMatch);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::rstrip(const std::string &s) {
   auto charMatch = [](char _c){
      return ::isspace(_c);
   };
   return strip(s, charMatch, true);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::strip(const std::string &s, const std::function<bool(char)>& charMatch, bool rtol) {
   //this is probably dumb and slow but, like me, it works
   if (s.empty()) return s;
   string retval;
   string t = s;
   if (!rtol) {
      std::reverse(t.begin(), t.end());
   }
   while (charMatch(t.back())){
      t.pop_back();
   }
   if (!rtol) {
      std::reverse(t.begin(), t.end());
   }
   return t;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::lrstrip(const std::string &s) {
   return string_tools::rstrip(string_tools::lstrip(s));
}

/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::count(const std::string &s, char c) {
   int count = 0;
   for (const auto& _c : s){
      if (_c == c){
         count++;
      }
   }
   return count;
}

/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::lcount(const std::string &s, char c) {
   int count = 0;
   for (const auto& _c : s){
      if (_c != c){
         break;
      }
      count++;
   }
   return count;
}

/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::rcount(const std::string &s, char c) {
   int count = 0;
   for (auto rit=s.rbegin(); rit!=s.rend(); ++rit) {
      const auto &_c = *rit;
      if (_c != c){
         break;
      }
      count++;
   }
   return count;
}

/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::countwhite(const std::string &s) {
   int count = 0;
   for (const auto& _c : s){
      if (::isspace(_c)){
         count++;
      }
   }
   return count;
}
/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::countwhitel(const std::string &s) {
   int count = 0;
   for (const auto& _c : s){
      if (!::isspace(_c)){
         break;
      }
      count++;
   }
   return count;
}
/////////////////////////////////////////////////////////////////////////////////////////
int string_tools::countwhiter(const std::string &s) {
   int count = 0;
   for (auto rit=s.rbegin(); rit!=s.rend(); ++rit) {
      const auto &_c = *rit;
      if (!::isspace(_c)){
         break;
      }
      count++;
   }
   return count;
}

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::fromList(const std::string &s) {
   auto lessLeftBracket = lstrip(s, '{');
   auto lessBrackets = rstrip(lessLeftBracket, '}');
   auto _split = split(lessBrackets, ",");
   vector<string> retval;
   for (const auto& element : _split){
      retval.push_back(lrstrip(element));
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::join(const std::string &s, const std::vector<std::string>& v) {
   if (v.empty()) return "";
   string retval;
   for (const auto& _v : v){
      retval += _v;
      retval += s;
   }
   for (const auto& c : s) {
      retval.pop_back();
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::listJoin(const std::vector<std::string> &v) {
   return "{" + join(", ", v) + "}";
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t string_tools::lcountUntil(const std::string &s, char c) {
   if (s.empty()) return 0;
   auto retval = 0;
   bool found = false;
   for (const auto& _c : s){
      if (_c == c) {
         found = true;
         break;
      }
      retval++;
   }
   if (!found){
      return string::npos;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t string_tools::rcountUntil(const std::string &s, char c) {
   if (s.empty()) return 0;
   auto retval = 0;
   bool found = false;
   for (auto it = s.rbegin(); it!=s.rend(); it++){
      if (*it == c) {
         found = true;
         break;
      }
      retval++;
   }
   if (!found){
      return string::npos;
   }
   return retval;
}