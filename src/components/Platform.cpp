#include "Platform.h"
#include <stdexcept>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#endif

#ifdef PLATFORM_LINUX
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#endif

//resist the urge to import std namespace BEFORE windows.h it will not work.
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getExePath() {
#ifdef PLATFORM_WINDOWS
   TCHAR szDir[MAX_PATH] = { 0 };
   GetModuleFileNameA(nullptr, szDir, MAX_PATH);

   TCHAR szPath[MAX_PATH] = {0};
   if (!GetModuleFileNameA(nullptr, szPath, MAX_PATH)) {
      auto error = GetLastError();
      LPTSTR errorText = nullptr;

      FormatMessage(
            // use system message tables to retrieve error text
            FORMAT_MESSAGE_FROM_SYSTEM
            // allocate buffer on local heap for error text
            | FORMAT_MESSAGE_ALLOCATE_BUFFER
            // Important! will fail otherwise, since we're not
            // (and CANNOT) pass insertion parameters
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &errorText,  // output
            0, // minimum size for output buffer
            nullptr);   // arguments - see note

      if (errorText) {
         string errString = string(errorText);
         LocalFree(errorText);
         errorText = nullptr;
         throw std::runtime_error("Platform: Windows: " + errString);
      }
   }
   auto retval = std::string(szPath, std::find(szPath, szPath + sizeof(szPath), '\0'));
   return retval;
#endif

#ifdef PLATFORM_LINUX
    char dest[PATH_MAX];
    memset(dest, 0, sizeof(dest)); // readlink does not null terminate!
    if (readlink("/proc/self/exe", dest, PATH_MAX) == -1) {
        perror("readlink");
    } else {
        printf("%s\n", dest);
        throw std::runtime_error("Platform Linux: unable to determine self exe path");
    }
    return string(dest, PATH_MAX);
#endif

}