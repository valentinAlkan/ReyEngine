#include "Logger.h"

using namespace std;
using namespace ReyEngine;

unique_ptr<Logger> ReyEngine::Logger::_self;
/////////////////////////////////////////////////////////////////////////////////////////
Logger &Logger::Logger::getInstance() {
   if (!_self){
      _self = unique_ptr<Logger>(new Logger);
   }
   return *_self;
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream::Stream(Logger& logger, const std::string& level)
: _logger(logger)
, _level(level)
{}


/////////////////////////////////////////////////////////////////////////////////////////
bool Logger::hasHistory(){
   return !Logger::getInstance()._history.empty();
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream::~Stream() {
   scoped_lock<mutex> lock(_logger._mutex);
   auto now = chrono::system_clock::now();
   auto now_time_t = chrono::system_clock::to_time_t(now);
   auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

   stringstream timestamp_ss;
   timestamp_ss << put_time(localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
   timestamp_ss << '.' << setfill('0') << setw(3) << now_ms.count();

   string timestamp = timestamp_ss.str();
   string log_message = "[" + timestamp + "] [" + _level + "] " + _ss.str();
   cout << log_message;
   fflush(stdout);
   _logger._history.push(log_message);
   if (_logger._history.size() > HISTORY_SIZE){
      _logger._history.pop();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::info() {
   return Stream(Logger::getInstance(), "INFO");
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::warn() {
   return Stream(Logger::getInstance(), "WARN");
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::error() {
   return Stream(Logger::getInstance(), "ERROR");
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::debug() {
   return Stream(Logger::getInstance(), "DEBUG");
}

/////////////////////////////////////////////////////////////////////////////////////////
string Logger::getFront() {
   scoped_lock<mutex> lock(getInstance()._mutex);
   string retval = getInstance()._history.front();
   getInstance()._history.pop();
   return retval;
}