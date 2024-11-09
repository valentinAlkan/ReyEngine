#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <queue>
#include <condition_variable>

namespace ReyEngine{
   class Logger {
      static constexpr long long HISTORY_SIZE = 128;
   public:
      class Stream {
      public:
         Stream(Logger& logger, const std::string& level);
         Stream(const Stream&) = delete;
         Stream& operator=(const Stream&) = delete;
         Stream(Stream&&) = default;

         template <typename T>
         Stream& operator<<(const T& msg) {
            _ss << msg;
            return *this;
         }

         // Overload for std::endl and other manipulators
         Stream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            _ss << manip;
            return *this;
         }

         ~Stream();
      private:
         Logger& _logger;
         std::string _level;
         std::stringstream _ss;
      };

      static Stream info();
      static Stream warn();
      static Stream error();
      static Stream debug();
      static std::string getFront();
      static bool hasHistory();
      static Logger& getInstance();
      ~Logger();
   private:
      Logger() = default;
      Logger(const Logger&) = delete;
      Logger& operator=(const Logger&) = delete;
      static std::unique_ptr<Logger> _self;

      std::queue<std::string> _history;
      std::mutex _mutex;
   };
}