#include "Platform.h"
#include <stdexcept>
#include <filesystem>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#include <shlobj.h>  // For SHGetFolderPath
#include <knownfolders.h>  // For FOLDERID constants
#include <shlwapi.h>  // For path manipulation
#endif

#ifdef PLATFORM_LINUX_BASED
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#endif

#ifdef PLATFORM_MACOS
#include <unistd.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#endif

//resist the urge to import std namespace BEFORE windows.h it will cause weird errors
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getExePath() {
#ifdef PLATFORM_WINDOWS
   CHAR szPath[MAX_PATH] = {0};
   if (!GetModuleFileNameA(nullptr, szPath, MAX_PATH)) {
      auto error = GetLastError();
      LPSTR errorText = nullptr;

      FormatMessageA(
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
            (LPSTR) &errorText,  // output
            0, // minimum size for output buffer
            nullptr);   // arguments - see note

      if (errorText) {
         string errString = string(errorText);
         LocalFree(errorText);
         errorText = nullptr;
         throw std::runtime_error("Platform: Windows: " + errString);
      }
      throw std::runtime_error("Platform: Windows: Failed to get module filename");
   }
   return std::string(szPath);
#endif

#ifdef PLATFORM_LINUX_BASED
   char dest[PATH_MAX];
    memset(dest, 0, sizeof(dest)); // readlink does not null terminate!
    ssize_t nbytes = readlink("/proc/self/exe", dest, PATH_MAX - 1);
    if (nbytes == -1) {
        perror("readlink");
        throw std::runtime_error("Platform Linux: unable to determine self exe path");
    }
    dest[nbytes] = '\0'; // Ensure null termination
    return string(dest);
#endif

#ifdef PLATFORM_MACOS
   char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        throw std::runtime_error("Platform macOS: unable to determine executable path");
    }
    return string(path);
#endif

   throw std::runtime_error("Platform: Unsupported platform for getExePath");
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getExeDir() {
   string exePath = getExePath();

#ifdef PLATFORM_WINDOWS
   // Find the last backslash or forward slash (Windows accepts both)
   size_t lastSlash = exePath.find_last_of("\\/");
#else
   // Find the last forward slash on Unix-like systems
    size_t lastSlash = exePath.find_last_of('/');
#endif

   if (lastSlash != string::npos) {
      return exePath.substr(0, lastSlash);
   }

   throw std::runtime_error("Unable to determine current exe directory!");
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getResourceDir() {
   return getExeDir() + REYENGINE_FILESYSTEM_PATH_SEP + "engine" + REYENGINE_FILESYSTEM_PATH_SEP + "resources";
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getFontsDir() {
   return getResourceDir() + REYENGINE_FILESYSTEM_PATH_SEP + "fonts";
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getIconsDir() {
   return getResourceDir() + REYENGINE_FILESYSTEM_PATH_SEP + "icons";
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getExeName() {
   string exePath = getExePath();

#ifdef PLATFORM_WINDOWS
   // Find the last backslash or forward slash (Windows accepts both)
   size_t lastSlash = exePath.find_last_of("\\/");
#else
   // Find the last forward slash on Unix-like systems
    size_t lastSlash = exePath.find_last_of('/');
#endif

   if (lastSlash != string::npos) {
      return exePath.substr(lastSlash + 1);
   }

   // If no path separator found, the whole string is the filename
   return exePath;
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getSystemTempDir() {
#ifdef PLATFORM_WINDOWS
   CHAR tempPath[MAX_PATH];
   DWORD result = GetTempPathA(MAX_PATH, tempPath);
   if (result == 0 || result > MAX_PATH) {
      throw std::runtime_error("Platform Windows: Failed to get temp directory");
   }
   return string(tempPath);
#endif

#ifdef PLATFORM_LINUX_BASED
   // Check environment variables in order of preference
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir && strlen(tmpdir) > 0) {
        return string(tmpdir);
    }

    tmpdir = getenv("TMP");
    if (tmpdir && strlen(tmpdir) > 0) {
        return string(tmpdir);
    }

    tmpdir = getenv("TEMP");
    if (tmpdir && strlen(tmpdir) > 0) {
        return string(tmpdir);
    }

    return "/tmp";
#endif

#ifdef PLATFORM_MACOS
   const char* tmpdir = getenv("TMPDIR");
    if (tmpdir && strlen(tmpdir) > 0) {
        return string(tmpdir);
    }
    return "/tmp";
#endif

   throw std::runtime_error("Platform: Unsupported platform for getSystemTempDir");
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getUserDir() {
#ifdef PLATFORM_WINDOWS
   CHAR userPath[MAX_PATH];
   if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userPath))) {
      return string(userPath);
   }

   // Fallback to environment variable
   const char* userProfile = getenv("USERPROFILE");
   if (userProfile) {
      return string(userProfile);
   }

   throw std::runtime_error("Platform Windows: Failed to get user directory");
#endif

#ifdef PLATFORM_LINUX_BASED
   // Try environment variable first
    const char* home = getenv("HOME");
    if (home && strlen(home) > 0) {
        return string(home);
    }

    // Fallback to passwd entry
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return string(pw->pw_dir);
    }

    throw std::runtime_error("Platform Linux: Failed to get user directory");
#endif

#ifdef PLATFORM_MACOS
   const char* home = getenv("HOME");
    if (home && strlen(home) > 0) {
        return string(home);
    }

    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return string(pw->pw_dir);
    }

    throw std::runtime_error("Platform macOS: Failed to get user directory");
#endif

   throw std::runtime_error("Platform: Unsupported platform for getUserDir");
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getUserLocalConfigDir() {
#ifdef PLATFORM_WINDOWS
   CHAR appDataPath[MAX_PATH];
   if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appDataPath))) {
      return string(appDataPath);
   }

   // Fallback to environment variable
   const char* localAppData = getenv("LOCALAPPDATA");
   if (localAppData) {
      return string(localAppData);
   }

   throw std::runtime_error("Platform Windows: Failed to get local app data directory");
#endif

#ifdef PLATFORM_LINUX_BASED
   // Follow XDG Base Directory Specification
    const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
    if (xdgConfigHome && strlen(xdgConfigHome) > 0) {
        return string(xdgConfigHome);
    }

    // Fallback to ~/.config
    string userDir = getUserDir();
    return userDir + "/.config";
#endif

#ifdef PLATFORM_MACOS
   string userDir = getUserDir();
    return userDir + "/Library/Application Support";
#endif

   throw std::runtime_error("Platform: Unsupported platform for getUserLocalConfigDir");
}

/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getUserLocalConfigDirRestrictedSecurity() {
#ifdef PLATFORM_WINDOWS
   // Define CSIDL_LOCAL_APPDATA_LOW if not available in MinGW
#ifndef CSIDL_LOCAL_APPDATA_LOW
#define CSIDL_LOCAL_APPDATA_LOW 0x0083
#endif

   CHAR appDataPath[MAX_PATH];
   if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA_LOW, nullptr, 0, appDataPath))) {
      return string(appDataPath);
   }

   // Fallback: try to construct LocalLow path manually
   CHAR profilePath[MAX_PATH];
   if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, profilePath))) {
      return string(profilePath) + "\\AppData\\LocalLow";
   }

   throw std::runtime_error("Platform Windows: Failed to get LocalLow directory");
#endif

#ifdef PLATFORM_LINUX_BASED
#endif

#ifdef PLATFORM_MACOS
#endif

   throw std::runtime_error("Platform: Unsupported platform for getUserLocalConfigDirRestrictedSecurity");
}


/////////////////////////////////////////////////////////////////////////////////////////
string CrossPlatform::getUserLocalConfigDirApp(){
   auto exeName = getExeName();
   constexpr string_view DEFAULT_EXE_NAME = ".exe";
   if (exeName.find(string(DEFAULT_EXE_NAME)) != string::npos){
      //remove .exe if it exists
      exeName = exeName.substr(0, exeName.size() - 4);
   }
   return getUserLocalConfigDir() + REYENGINE_FILESYSTEM_PATH_SEP + exeName;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string CrossPlatform::getUserLocalConfigDirAppFile() {
   return getUserLocalConfigDirApp() + REYENGINE_FILESYSTEM_PATH_SEP + ".config";
}

/////////////////////////////////////////////////////////////////////////////////////////
vector<string> CrossPlatform::getRootFolders() {
   std::vector<std::string> roots;

#ifdef PLATFORM_WINDOWS
   // Windows: Get drive letters
   DWORD drives = GetLogicalDrives();
   for (char drive = 'A'; drive <= 'Z'; ++drive) {
      if (drives & (1 << (drive - 'A'))) {
         std::string driveStr = std::string(1, drive) + ":\\";
         roots.push_back(driveStr);
      }
   }

#elif IS_LINUX()
   roots.emplace_back("/");
#elif IS_MACOS()
    // Other Unix-like systems might need different approaches
    roots.push_back("/"); // At minimum, root exists
#endif

   return roots;
}