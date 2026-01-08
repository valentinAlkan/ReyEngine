#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <queue>
#include <condition_variable>
#include <string>
#include <functional>
#include <streambuf>
#include <ostream>
#include <utility>

namespace ReyEngine{
   class CustomLogger;
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
      static Stream exit();
      Stream log(const std::string& logLevel="");
      static std::string getFront();
      static bool hasHistory();
      static Logger& getInstance(){static Logger _instance("GlobalLogger", std::cout); return _instance;}
      ~Logger();
      Logger(const Logger&) = delete;
      Logger& operator=(const Logger&) = delete;
   protected:
      std::ostream& _out;
      Logger(const std::string& name, std::ostream&);
   private:
      std::queue<std::string> _history;
      const std::string _name;
      std::mutex _mutex;
      friend class Stream;
   };

   namespace Internal {
      class CallbackStreambuf : public std::streambuf {
      public:
         using Callback = std::function<void(const std::string&, bool)>;

         CallbackStreambuf(Callback callback)
         : callback_(std::move(callback)) {}

         ~CallbackStreambuf() override {
            // Ensure any remaining data is flushed on destruction
            if (!buffer_.empty()) {
               sync();
            }
         }

      protected:
         std::streamsize xsputn(const char* s, std::streamsize n) override {
            buffer_.append(s, n);
            process_buffer();
            return n;
         }

         int_type overflow(int_type ch) override {
            if (ch != traits_type::eof()) {
               buffer_ += static_cast<char>(ch);
               process_buffer();
            }
            return ch;
         }

         int sync() override {
            // If there is any data left in the buffer, it's a partial line
            // that needs to be sent because of the flush.
            if (!buffer_.empty()) {
               callback_(buffer_, true);
               buffer_.clear();
            } else {
               callback_("", true);
            }
            return 0;
         }

      private:
         void process_buffer() {
            size_t newline_pos;
            while ((newline_pos = buffer_.find('\n')) != std::string::npos) {
               // Found a complete line.
               // Send it to the callback, indicating it's a regular line-break.
               callback_(buffer_.substr(0, newline_pos + 1), false);
               buffer_.erase(0, newline_pos + 1);
            }
         }

         Callback callback_;
         std::string buffer_;
      };
   }

   class CallbackStream : public std::ostream {
   public:
      explicit CallbackStream(std::function<void(const std::string&, bool)> callback)
      : std::ostream(&_streambuf), _streambuf(std::move(callback)) {}
   private:
      Internal::CallbackStreambuf _streambuf;
   };

   class CustomLogger : public Logger {
   public:
      CustomLogger(const std::string& name, std::ostream& os)
      : Logger(name, os)
      {};
   private:
      //disable these static functions
      static Stream info();
      static Stream warn();
      static Stream error();
      static Stream debug();
      static Stream exit();
   };

}