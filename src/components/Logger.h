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
      Stream log(const std::string& logLevel);
      static std::string getFront();
      static bool hasHistory();
      static Logger& getInstance();
      ~Logger();
      Logger(const Logger&) = delete;
      Logger& operator=(const Logger&) = delete;
      static std::unique_ptr<Logger> customLogger(std::ostream&);
   protected:
      std::ostream& _out;
   private:
      Logger(std::ostream&);
      static std::unique_ptr<Logger> _self;

      std::queue<std::string> _history;
      std::mutex _mutex;
      friend class Stream;
   };


   namespace Internal {
      class CallbackStreambuf : public std::streambuf {
      public:
         CallbackStreambuf(std::function<void(const std::string &)> callback)
         : callback_(std::move(callback)) {}
         ~CallbackStreambuf() override { sync(); }
      protected:
         std::streamsize xsputn(const char *s, std::streamsize n) override {
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
            if (!buffer_.empty()) {
               callback_(buffer_);
               buffer_.clear();
            }
            return 0; // Success
         }

      private:
         // Processes the internal buffer, sending complete lines to the callback.
         void process_buffer() {
            size_t newline_pos;
            while ((newline_pos = buffer_.find('\n')) != std::string::npos) {
               // Found a newline, send the line (including newline) to the callback.
               callback_(buffer_.substr(0, newline_pos + 1));
               // Erase the sent part from the buffer.
               buffer_.erase(0, newline_pos + 1);
            }
         }

         std::function<void(const std::string &)> callback_;
         std::string buffer_;
      };
   }

   class CallbackStream : public std::ostream {
   public:
      explicit CallbackStream(std::function<void(const std::string&)> callback)
      // The initializer list constructs the ostream base with a pointer
      // to our streambuf member, and then constructs the streambuf itself.
      : std::ostream(&streambuf_), streambuf_(std::move(callback)) {}
   private:
      Internal::CallbackStreambuf streambuf_;
   };
}