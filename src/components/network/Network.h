#pragma once
#include "Platform.h"
#include <string>
#include <iostream>

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
#define SOCKFD unsigned int
#endif

//make WSA err readable
struct NetworkError {
    NetworkError();
    std::string msg;
    int err;
};

struct addrinfo;
struct sockaddr_in;
namespace Sockets {
   //addrinfo helper
   struct AddressInfo{
      ~AddressInfo();
      AddressInfo(const std::string& addr, uint32_t port);
      int                 _port;
      std::string         _strAddr;
      std::string         _strPort;
      addrinfo*           _addrinfo;
      char                _decimal_port[16];
      static std::string parseIp(sockaddr_in* srcaddr);
      static int parsePort(sockaddr_in* srcaddr);
   };

   struct HostInfo {
      HostInfo(const std::string& ip, int port);
      HostInfo(sockaddr_in* srcaddr);
      const std::string ip;
      const int port;
      friend std::ostream& operator<<(std::ostream& os, const HostInfo& host) {return os << host.ip << ":" << host.port;}
   };
}