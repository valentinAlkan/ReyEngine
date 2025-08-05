#pragma once
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#else
#define PLATFORM_LINUX
#endif

namespace CrossPlatform{
   std::string getExePath();
   std::string getExeDir();
   std::string getExeName();
}