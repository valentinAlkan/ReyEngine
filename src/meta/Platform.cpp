#include "Platform.h"
#include <stdexcept>
#include <algorithm>

#ifdef REYENGINE_PLATFORM_WINDOWS
#include "windows.h"
#endif

using namespace std;

#ifdef REYENGINE_PLATFORM_LINUX
/////////////////////////////////////////////////////////////////////////////////////////
std::string getExecutablePath() {
   char rawPathName[PATH_MAX];
   realpath(PROC_SELF_EXE, rawPathName);
   return  std::string(rawPathName);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string mergePaths(std::string pathA, std::string pathB) {
  return pathA+"/"+pathB;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getExePath() {
#ifdef REYENGINE_PLATFORM_WINDOWS
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

#ifdef REYENGINE_PLATFORM_LINUX
   todo: needs testing

    std::string executablePath = getExecutablePath();
    char *executablePathStr = new char[executablePath.length() + 1];
    strcpy(executablePathStr, executablePath.c_str());
    char* executableDir = dirname(executablePathStr);
    delete [] executablePathStr;
    return std::string(executableDir);
}

#endif

}