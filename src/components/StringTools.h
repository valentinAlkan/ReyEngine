#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace string_tools{
   std::vector<std::string> split(const std::string& s, const std::string& delimString);
   std::vector<std::string> split(const std::string& s, char delimChar);
   std::vector<std::string> split(const std::string& s); //split all whitespace
   std::string lstrip(const std::string& s, char c); //strips char(s) c from the left
   std::string lstrip(const std::string& s); //strips whitespace from the left
   std::string rstrip(const std::string& s, char c); //strips char(s) c from the right
   std::string rstrip(const std::string& s); //strips whitespace from the right
   std::string strip(const std::string& s, const std::function<bool(char c)>& charMatch, bool rtol = false);
   std::string lrstrip(const std::string& s); //strips whitespace from the right and left
   std::string extractNumeric(const std::string& s); //pull out all numeric chars and return them, in order
   std::string extractNumericl(const std::string& s); //pull out all numeric chars on the left side of the string, and return them, in order. stop when encountering a non-numeric char.
   std::string extractNumericr(const std::string& s); //pull out all numeric chars on the right side of the string, and return them, in order. stop when encountering a non-numeric char.
   std::string extractNonNumeric(const std::string& s); //pull out all non-numeric chars and return them, in order
   std::string extractNonNumericl(const std::string& s); //pull out all non-numeric chars on the left side of the string, and return them, in order. stop when encountering a numeric char.
   std::string extractNonNumericr(const std::string& s); //pull out all non-numeric chars on the right side of the string, and return them, in order. stop when encountering a numeric char.
   int count(const std::string& s, char c); //counts ocurrences of a character in a string
   int lcount(const std::string& s, char c); //counts ocurrences of a character in a string until non-match, left start
   int rcount(const std::string& s, char c); //counts ocurrences of a character in a string until non-match, right start
   std::optional<size_t> lcountUntil(const std::string& s, char c); //counts number of characters in a string until match, left start
   std::optional<size_t> rcountUntil(const std::string& s, char c); //counts number of characters in a string until match, right start
   std::optional<std::pair<size_t, size_t>> decimalCount(const std::string& numeric); //returns a pair of size_t's, char count before decimal and after, respectively. Optionally null if non-real number or null string.
   std::pair<size_t, size_t> decimalCount(double d); //more optimized than the above since we assume its a number to begin with
   int countwhitel(const std::string& s);
   int countwhiter(const std::string& s);
   int countwhite(const std::string& s);
   std::string join(const std::string& s, const std::vector<std::string>& v);
   std::string join(char c, const std::vector<std::string>& v);
   template <typename T>
   std::string listJoin(const std::vector<T>& v); //join into an array-like list
   std::vector<std::string> fromList(const std::string& s); //converts "{ARRAYS, OF, STRINGS} to vec<string>("ARRAYS", "OF", "STRINGS"). Strips leading and trailing whitespace on all elements.
   std::optional<float> isNumeric(const std::string& s); //true if the value can be represented as a float
   std::optional<int> isInteger(const std::string& s); //true if the value can be represented as an int
   std::string toUpper(const std::string& s);
   std::string toLower(const std::string& s);
   bool contains(const std::string& s, const std::string& substr);
   std::pair<std::string, std::string> nFront(const std::string& s, size_t n); //returns first n characters of string, in order
   std::pair<std::string, std::string> nBack(const std::string& s, size_t n); //returns last n characters of string, in order
   std::optional<std::tuple<std::string, std::string, std::string>> nAt(const std::string& s, size_t n, size_t pos); //returns n characters of string, in order, starting at pos. Nullopt on bad params

    namespace AnsiColor{
        namespace Foreground {
            std::string EscapeBlack(std::string_view str);
            std::string EscapeRed(std::string_view str);
            std::string EscapeGreen(std::string_view str);
            std::string EscapeYellow(std::string_view str);
            std::string EscapeBlue(std::string_view str);
            std::string EscapeMagenta(std::string_view str);
            std::string EscapeCyan(std::string_view str);
            std::string EscapeWhite(std::string_view str);
        }
        namespace EscapeCodes {
            static constexpr char Reset[] = "\033[0m";
            static constexpr char Black[] = "\u001b[30m";
            static constexpr char Red[] = " \u001b[31m";
            static constexpr char Green[] = " \u001b[32m";
            static constexpr char Yellow[] = " \u001b[33m";
            static constexpr char Blue[] = " \u001b[34m";
            static constexpr char Magenta[] = " \u001b[35m";
            static constexpr char Cyan[] = " \u001b[36m";
            static constexpr char White[] = " \u001b[37m";
        }
    }
}