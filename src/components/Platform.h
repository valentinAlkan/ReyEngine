#pragma once
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define PLATFORM_WINDOWS
#ifdef _WIN64
#define PLATFORM_WINDOWS_64
#else
#define PLATFORM_WINDOWS_32
#endif
#elif defined(__APPLE__)
#include <TargetConditionals.h>
    #define PLATFORM_APPLE
    #if TARGET_IPHONE_SIMULATOR
        #define PLATFORM_IOS_SIMULATOR
    #elif TARGET_OS_IPHONE
        #define PLATFORM_IOS
    #elif TARGET_OS_MAC
        #define PLATFORM_MACOS
    #else
        #define PLATFORM_APPLE_UNKNOWN
    #endif
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
    #define PLATFORM_LINUX_BASED
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    #define PLATFORM_LINUX
    #define PLATFORM_LINUX_BASED
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #define PLATFORM_BSD
    #define PLATFORM_UNIX_LIKE
#elif defined(__unix__) || defined(__unix) || defined(unix)
    #define PLATFORM_UNIX
    #define PLATFORM_UNIX_LIKE
#elif defined(__CYGWIN__)
    #define PLATFORM_CYGWIN
    #define PLATFORM_UNIX_LIKE
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #define PLATFORM_MINGW
    #define PLATFORM_WINDOWS_LIKE
#else
    #define PLATFORM_UNKNOWN
#endif

// Architecture detection
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64)
#define ARCH_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386) || defined(i386)
#define ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ARM32
#else
    #define ARCH_UNKNOWN
#endif

// Compiler detection
#if defined(_MSC_VER)
#define COMPILER_MSVC
static_assert(false, "ReyEngine does not support MSVC")
#elif defined(__clang__)
#define COMPILER_CLANG
static_assert(false, "ReyEngine does not support CLANG")
#elif defined(__GNUC__)
#define COMPILER_GCC
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_MINGW
static_assert(false, "ReyEngine does not support MINGW")
#else
    #define COMPILER_UNKNOWN
    static_assert(false, "ReyEngine does not support this compiler")
#endif

#define IS_WINDOWS() defined(PLATFORM_WINDOWS)
#define IS_LINUX() defined(PLATFORM_LINUX)
#define IS_MACOS() defined(PLATFORM_MACOS)
#define IS_ANDROID() defined(PLATFORM_ANDROID)
#define IS_IOS() defined(PLATFORM_IOS)
#define IS_UNIX_LIKE() defined(PLATFORM_UNIX_LIKE)

namespace CrossPlatform{
   std::string getExePath();
   std::string getExeDir();
   std::string getExeName();
   std::string getSystemTempDir(); //returns the global system temp directory - not necessarily user writeable
   std::string getUserDir(); //returns the current user's home directory - necessarily user writable
   std::string getUserLocalConfigDir(); //returns a location suitable for storing per-app data
   std::string getUserLocalConfigDirApp(); //returns a directory string in the local config dir with the same name as this application
   std::string getUserLocalConfigDirRestrictedSecurity(); //something like %APPDATA%/LocalLow in windows
   std::vector<std::string> getRootFolders(); //returns / for unix-like systems, a vector of drive letters in windows
}