#pragma once

#include <iostream>
#include <sstream>
#include "SystemTime.h"

class Logger: public std::ostream
{
   class LoggerStreamBuf: public std::stringbuf {
      std::ostream&     _output;
      std::string       _prefix;
   public:
      LoggerStreamBuf(std::ostream& str, std::string prefix)
      : _output(str)
      , _prefix(std::move(prefix)){}
      ~LoggerStreamBuf() {
         if (pbase() != pptr()) {
            putOutput();
         }
      }

      virtual int sync() {
         putOutput();
         return 0;
      }
      void putOutput() {
         _output << "[" << SystemTime::nowSystemString() << "] [" << _prefix << "] : " << str();
         str("");
         _output.flush();
      }
   };

   // My Stream just uses a version of my special buffer
   LoggerStreamBuf _buffer;
public:
   Logger(std::ostream& str, std::string prefix)
         : std::ostream(&_buffer)
         , _buffer(str, std::move(prefix))
   {}
};
