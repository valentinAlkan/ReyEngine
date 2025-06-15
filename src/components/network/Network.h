#pragma once
#include "Platform.h"
#include <string>

#ifdef PLATFORM_WINDOWS
// ! WinSock2 MUST be included before winsock! you'll get a shitload of redefiniton errors if you don't
#undef byte
#include <memory>
#include <string>
#define SOCKFD unsigned long long
#define close(x) closesocket(x)
namespace WinNet {
   class WinSockInit {
      WinSockInit();
      ~WinSockInit();
   public:
      static WinSockInit& instance(){
         static WinSockInit winnit;
         return winnit;
      }
   public:
      WinSockInit(WinSockInit const&)    = delete;
      void operator=(WinSockInit const&) = delete;
   };
}


#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif


#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#endif

//make WSA err readable
struct NetworkError {
    NetworkError();
    std::string msg;
    int err;
};