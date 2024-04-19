#pragma once
#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define REYENGINE_PLATFORM_WINDOWS
#else
#define REYENGINE_PLATFORM_LINUX
#endif

namespace CrossPlatform{
   std::string getExePath();
}