#include "Logger.h"

#include <memory>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger(const std::string& name, std::ostream& outdevice)
: _out(outdevice)
, _name(name)
{}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::~Logger(){
   // can trigger a lot of nastiness if you don't use the global logger here.
   // Custom loggers and callback streams in particular might react very weirdly
   Logger::exit() << "Shutting down logger " << _name << endl;
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
   string log_message = "[" + timestamp + "] ";
   if (!_level.empty()) log_message += "[" + _level + "] ";
   log_message += _ss.str();
   _logger._out << log_message << std::flush;
   _logger._history.push(log_message);
   if (_logger._history.size() > HISTORY_SIZE){
      _logger._history.pop();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::info() {
   return {Logger::getInstance(), "INFO"};
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::warn() {
   return {Logger::getInstance(), "WARN"};
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::error() {
   return {Logger::getInstance(), "ERROR"};
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::debug() {
   return {Logger::getInstance(), "DEBUG"};
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::exit() {
   return {Logger::getInstance(), "EXIT"};
}

/////////////////////////////////////////////////////////////////////////////////////////
Logger::Stream Logger::log(const std::string& logLevel) {
   return {*this, logLevel};
}

/////////////////////////////////////////////////////////////////////////////////////////
string Logger::getFront() {
   scoped_lock<mutex> lock(getInstance()._mutex);
   string retval = getInstance()._history.front();
   getInstance()._history.pop();
   return retval;
}