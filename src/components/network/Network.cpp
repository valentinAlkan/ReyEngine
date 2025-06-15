#include "Network.h"
#include <sstream>
#include <iostream>
#include "StringTools.h"
#ifdef PLATFORM_WINDOWS
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#include <ws2def.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#endif

using namespace std;
#ifdef PLATFORM_WINDOWS
using namespace WinNet;

WinSockInit::WinSockInit(){
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != NO_ERROR) {
        cerr << "WSAStartup failed with error " << res << endl;
    }
}
WinSockInit::~WinSockInit(){
    WSACleanup();
}

#endif
NetworkError::NetworkError() {
    char msgbuf[1024];   // for a message up to 255 bytes.
    msgbuf[0] = 0;
#ifdef PLATFORM_WINDOWS
    err = WSAGetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                  NULL,                // lpsource
                  err,                 // message id
                  MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                  msgbuf,              // output buffer
                  sizeof(msgbuf),     // size of msgbuf, bytes
                  NULL);               // va_list of arguments
    if (msgbuf[0]) {
        msg = std::string(msgbuf);
        msg = string_tools::rstrip(msg);
    }
#elif defined(PLATFORM_LINUX)
    err = errno;
    if (strerror(err)){
        msg = strerror(err);
    }
#endif
}

using namespace Sockets;
////////////////////////////////////////////////////////////////////////////////////////
AddressInfo::AddressInfo(const std::string& addr, uint32_t port)
: _port(port)
, _strAddr(addr)
{
#ifdef PLATFORM_WINDOWS
   //load winsock
   WinNet::WinSockInit::instance();
#endif

   snprintf(_decimal_port, sizeof(_decimal_port), "%d", _port);
   _decimal_port[sizeof(_decimal_port) / sizeof(_decimal_port[0]) - 1] = '\0';

   addrinfo hints{};
   hints.ai_family = AF_INET;        // Force IPv4
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_protocol = IPPROTO_UDP;

   int r = getaddrinfo(addr.c_str(), _decimal_port, &hints, &_addrinfo);
   if(r != 0 || _addrinfo == nullptr){
      NetworkError e;
      cerr << "UDPSocket AddressInfo error :  " << e.err << " : " << e.msg << endl;
      cerr << "getaddrinfo error code: " << r << endl;
      throw std::runtime_error("invalid address or port for UDP socket: \"" + addr + ":" + _decimal_port + "\"");
   }
}

////////////////////////////////////////////////////////////////////////////////////////
AddressInfo::~AddressInfo(){
   freeaddrinfo(_addrinfo);
};

////////////////////////////////////////////////////////////////////////////////////////
std::string AddressInfo::parseIp(sockaddr_in* srcaddr) {
   char ip_str[INET_ADDRSTRLEN];

   if (inet_ntop(AF_INET, &(srcaddr->sin_addr), ip_str, INET_ADDRSTRLEN) != nullptr) {
      return std::string(ip_str);
   }

   //fallback
   unsigned char addrBytes[4];
   addrBytes[0] = srcaddr->sin_addr.s_addr & 0xFF;
   addrBytes[1] = (srcaddr->sin_addr.s_addr >> 8) & 0xFF;
   addrBytes[2] = (srcaddr->sin_addr.s_addr >> 16) & 0xFF;
   addrBytes[3] = (srcaddr->sin_addr.s_addr >> 24) & 0xFF;

   std::stringstream ss;
   ss << static_cast<int>(addrBytes[0]) << "."
      << static_cast<int>(addrBytes[1]) << "."
      << static_cast<int>(addrBytes[2]) << "."
      << static_cast<int>(addrBytes[3]);
   return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////
int AddressInfo::parsePort(sockaddr_in* srcaddr) {
   // Convert from network byte order to host byte order
   return ntohs(srcaddr->sin_port);
}

////////////////////////////////////////////////////////////////////////////////////////
HostInfo::HostInfo(const std::string& ip, int port)
: ip(ip)
, port(port)
{}

////////////////////////////////////////////////////////////////////////////////////////
HostInfo::HostInfo(sockaddr_in *srcaddr)
: ip(AddressInfo::parseIp(srcaddr))
, port(AddressInfo::parsePort(srcaddr))
{}