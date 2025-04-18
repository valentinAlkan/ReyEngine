#include "StringTools.h"
#include "FileSystem.h"
#include <algorithm>

using namespace string_tools;
using namespace AnsiColor;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::pathSplit(const std::string& s) {
   vector<string> retval;
   string token;
   for (auto c : s){
      if (c == ReyEngine::FileSystem::_PATH_SEP_WIN || c == ReyEngine::FileSystem::_PATH_SEP_OTHER){
         retval.push_back(token);
         token.clear();
         continue;
      }
      token += c;
   }
   retval.push_back(token);
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::pathJoin(const std::vector<std::string>& v) {
   std::string retval;
   for (const auto& token : v){
      retval += ReyEngine::FileSystem::_PATH_SEP_OTHER + token;
   }
   if (retval.empty()){
      retval += ReyEngine::FileSystem::_PATH_SEP_OTHER;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::split(const std::string& s, const std::string& delimString) {
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
vector<string> string_tools::split(const std::string &s, char delimChar) {
   auto cstr = string() + delimChar;
   return split(s, cstr);
}

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> string_tools::split(const std::string &s) {
    vector<string> retval;
    size_t start = 0;
    string substr = s;
    size_t found;
    auto find = [&](const string& s){
        size_t i = 0;
        for (const auto& c : s){
            if (isspace(c)) {found = i; return i;}
            i++;
        }
        found = string::npos;
        return string::npos;
    };
    while(find(s) != string::npos){
        retval.emplace_back(s, start, found);
        start += found + 1;
        if (start >= s.size()){
            break;
        }
        substr = string(s, start, s.size() - start);
    }
    if (retval.empty()) retval.push_back(substr);
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
std::string string_tools::extractNumeric(const std::string &s) {
    string retval;
    for (char c : s){
        if (isdigit(c)) retval.push_back(c);
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::extractNumericl(const std::string &s) {
    string retval;
    for (char c : s){
        if (isdigit(c)){
            retval.push_back(c);
        } else {
            break;
        }
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::extractNumericr(const std::string &s) {
    string s_reversed = s;
    std::reverse(s_reversed.begin(), s_reversed.end());
    string retval;
    for (char c : s_reversed){
        if (isdigit(c)){
            retval.insert(retval.begin(), c);
        } else {
            break;
        }
    }
    return retval;
}


/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::extractNonNumeric(const std::string &s) {
    string retval;
    for (char c : s){
        if (!isdigit(c)) retval.push_back(c);
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::extractNonNumericl(const std::string &s) {
    string retval;
    for (char c : s){
        if (!isdigit(c)){
            retval.push_back(c);
        } else {
            break;
        }
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::extractNonNumericr(const std::string &s) {
    string s_reversed = s;
    std::reverse(s_reversed.begin(), s_reversed.end());
    string retval;
    for (char c : s_reversed){
        if (!isdigit(c)){
            retval.insert(retval.begin(), c);
        } else {
            break;
        }
    }
    return retval;
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
   //there will be an extra string on the end, pop it off char by char (prolly theres a more efficient way to do this?)
   for (const auto& c : s) {
      retval.pop_back();
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::join(char c, const std::vector<std::string>& v) {
   if (v.empty()) return "";
   string retval;
   for (const auto& _v : v){
      retval += _v;
      retval += c;
   }

   //pop off extra char
   retval.pop_back();
   return retval;
}


/////////////////////////////////////////////////////////////////////////////////////////
template <>
std::string string_tools::listJoin(const std::vector<std::string> &v) {
   return "{" + join(", ", v) + "}";
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> string_tools::lcountUntil(const std::string &s, char c) {
   if (s.empty()) return nullopt;
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
      return nullopt;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<size_t> string_tools::rcountUntil(const std::string &s, char c) {
   if (s.empty()) return nullopt;
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
      return nullopt;
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::pair<size_t, size_t>> string_tools::decimalCount(const std::string &numeric) {
   //enforce a real number with only one scan
   if (numeric.empty()) return nullopt;
   auto decimalCount = 0;
   auto integerCount = 0;
   bool foundDecimal = false;
   for (auto it = numeric.begin(); it!=numeric.end(); it++){
      auto c = *it;
      //must be a number
      if (c == '.') {
         if (foundDecimal) return nullopt; //non-real number
         foundDecimal = true;
         continue;
      } else {
         if (!::isdigit(c) && c != '+' && c != '-') return nullopt;
         foundDecimal ? decimalCount++ : integerCount++;
      }
   }
   return std::pair<size_t, size_t>(integerCount, decimalCount);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::pair<size_t, size_t> string_tools::decimalCount(double d) {
   auto decimalCount = 0;
   auto integerCount = 0;
   bool foundDecimal = false;
   auto numeric = to_string(d);
   for (auto it = numeric.begin(); it!=numeric.end(); it++){
      if (*it == '.') {
         foundDecimal = true;
         continue;
      } else {
         foundDecimal ? integerCount++ : decimalCount++;
      }
   }
   return {integerCount, decimalCount};
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<float> string_tools::isNumeric(const std::string &s) {
    try {
        return std::stof(s);
    } catch (...){
        return nullopt;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<int> string_tools::isInteger(const std::string &s) {
    try {
        return std::stoi(s);
    } catch (...){
        return nullopt;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::toUpper(const std::string &s) {
    string retval;
    for (auto& c: s) retval += std::toupper(c);
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string string_tools::toLower(const std::string &s) {
    string retval;
    for (auto& c: s) retval += std::tolower(c);
    return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool string_tools::contains(const std::string &s, const std::string& substr) {
    return s.find(substr) != string::npos;
}

std::string AnsiColor::Foreground::EscapeBlack(std::string_view str){return std::string(EscapeCodes::Black) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeRed(std::string_view str){return std::string(EscapeCodes::Red) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeGreen(std::string_view str){return std::string(EscapeCodes::Green) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeYellow(std::string_view str){return std::string(EscapeCodes::Yellow) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeBlue(std::string_view str){return std::string(EscapeCodes::Blue) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeMagenta(std::string_view str){return std::string(EscapeCodes::Magenta) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeCyan(std::string_view str){return std::string(EscapeCodes::Cyan) + std::string(str) + std::string(EscapeCodes::Reset);}
std::string AnsiColor::Foreground::EscapeWhite(std::string_view str){return std::string(EscapeCodes::White) + std::string(str) + std::string(EscapeCodes::Reset);}