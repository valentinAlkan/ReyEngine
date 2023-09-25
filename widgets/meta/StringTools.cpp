#include "StringTools.h"

using namespace string_tools;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::split(const std::string &s, const std::string &delimString) {
   vector<string> retval;
   size_t start = 0;
   for (size_t pos=0; pos<s.size(); pos++){
      auto substr = string(s, start, pos);
      if (substr.find(delimString) == 0){
         retval.emplace_back(s, start, pos);
         pos += delimString.size();
      }
   }
   if (retval.empty()){
      retval.push_back(s);
   }
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
   return strip(s, charMatch);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::strip(const std::string &s, const std::function<bool(char)>& charMatch, bool rtol) {
   //this is probably dumb and slow but, like me, it works
   string retval;
   string t = s;
   if (!rtol) {
      //we want the string reversed normally so we can pop back
      std::reverse(t.begin(), t.end());
   }
   while (charMatch(t.back())){
      t.pop_back();
   }
   std::reverse(t.begin(), t.end());
   return t;
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
