#include "Network.h"
#include "UDP.h"
#include "Logger.h"
#include <sstream>
#include <iostream>
#ifdef PLATFORM_WINDOWS
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#elif defined(PLATFORM_LINUX)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#endif

using namespace std;
using namespace Sockets;
using namespace ReyEngine;

UDPSocket::AddressInfo::AddressInfo(const std::string& addr, uint32_t port)
: _port(port)
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
      Logger::error() << "UDPSocket AddressInfo error :  " << e.err << " : " << e.msg << endl;
      Logger::error() << "getaddrinfo error code: " << r << endl;
      throw UDPRuntimeError(("invalid address or port for UDP socket: \"" + addr + ":" + _decimal_port + "\"").c_str());
   }
}

////////////////////////////////////////////////////////////////////////////////////////
UDPSocket::AddressInfo::~AddressInfo(){
   freeaddrinfo(_addrinfo);
};

////////////////////////////////////////////////////////////////////////////////////////
UDPSocket::UDPSocket(const std::string& addr, uint32_t port, bool so_reuse){
   last_srcaddr = new sockaddr_in;
   bind(addr, port, so_reuse);
}

////////////////////////////////////////////////////////////////////////////////////////
UDPSocket::~UDPSocket(){
   if (_isValid) {
      close(sockfd);
   }
   delete last_srcaddr;
}

////////////////////////////////////////////////////////////////////////////////////////
void UDPSocket::bind(const std::string& addr, uint32_t port, bool reuseAddr){
#ifdef PLATFORM_WINDOWS
   WinNet::WinSockInit::instance();
#endif
   // Create socket
   sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if(sockfd == -1) {
      NetworkError e;
      Logger::error() << "UDPSocket socket() creation failed, errno " << e.err << " : " << e.msg << endl;
      throw UDPRuntimeError(("could not create UDP socket for: \"" + addr + "\", error: " + e.msg).c_str());
   }

   // Set SO_REUSEADDR if requested
   if (reuseAddr) {
      int opt = 1;
#ifdef PLATFORM_WINDOWS
      int retval = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#elif defined(PLATFORM_LINUX)
      int retval = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
      if (retval < 0) {
         NetworkError e;
         Logger::error() << "UDPSocket setsockopt SO_REUSEADDR errno " << e.err << " : " << e.msg << endl;
         close(sockfd);
         throw UDPRuntimeError("could not set SO_REUSEADDR");
      }
   }

   // Prepare bind address
   sockaddr_in bind_addr{};
   bind_addr.sin_family = AF_INET;
   bind_addr.sin_port = htons(port);  // htons(0) for auto-assign

   // Handle address
   if (addr == "0.0.0.0" || addr.empty()) {
      bind_addr.sin_addr.s_addr = INADDR_ANY;
   } else if (addr == "localhost") {
      bind_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   } else {
      if (inet_pton(AF_INET, addr.c_str(), &bind_addr.sin_addr) != 1) {
         close(sockfd);
         throw UDPRuntimeError(("invalid address: \"" + addr + "\"").c_str());
      }
   }

   // Bind socket
   if (::bind(sockfd, (sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
      NetworkError e;
      Logger::error() << "UDPSocket Bind errno " << e.err << " : " << e.msg << endl;
      close(sockfd);
      throw UDPRuntimeError(("could not bind UDP socket with: \"" + addr + ":" + std::to_string(port) + "\"").c_str());
   }

   // Get the actual bound address and port (important for port 0)
   socklen_t addr_len = sizeof(bind_addr);
   if (getsockname(sockfd, (sockaddr*)&bind_addr, &addr_len) == 0) {
      uint32_t actual_port = ntohs(bind_addr.sin_port);

      // Create AddressInfo with the actual bound port
      bindAddr = make_unique<AddressInfo>(addr, actual_port);

      if (port == 0) {
         Logger::info() << "UDPSocket: System assigned port " << actual_port << endl;
      }
   } else {
      // Fallback: create AddressInfo with requested port
      bindAddr = make_unique<AddressInfo>(addr, port);
   }

   _isValid = true;
 }

////////////////////////////////////////////////////////////////////////////////////////
 int UDPSocket::getPort() const {
     return bindAddr->_port;
 }

////////////////////////////////////////////////////////////////////////////////////////
 std::string UDPSocket::getAddr() const {
     return bindAddr->_strAddr;
 }

////////////////////////////////////////////////////////////////////////////////////////
 int UDPSocket::recv(char* recvBuf, size_t max_size){
   // our socket will already have data
   socklen_t addrlen = sizeof(struct sockaddr_in);
   return ::recvfrom(sockfd, recvBuf, (int)max_size, 0, (sockaddr*)last_srcaddr, &addrlen);
}

////////////////////////////////////////////////////////////////////////////////////////
 HostInfo UDPSocket::getRecvAddr() const {
     return {parseClientIP(last_srcaddr), parseClientPort(last_srcaddr)};
 }

////////////////////////////////////////////////////////////////////////////////////////
void UDPSocket::setSendAddr(const std::string& host, uint32_t port) {
   sendAddr = make_unique<AddressInfo>(host, port);
}

////////////////////////////////////////////////////////////////////////////////////////
std::string UDPSocket::parseClientIP(sockaddr_in* srcaddr) const {
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
int UDPSocket::parseClientPort(sockaddr_in* srcaddr) const {
   // Convert from network byte order to host byte order
   return ntohs(srcaddr->sin_port);
}

////////////////////////////////////////////////////////////////////////////////////////
void UDPSocket::send(const char* buf, size_t size) {
   // Send data to a specific address
   sendto(sockfd, buf, size, 0, sendAddr->_addrinfo->ai_addr, static_cast<int>(sendAddr->_addrinfo->ai_addrlen));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void UDPListener::listen(const std::string& addr, uint32_t port, bool so_reuse) {
   _sockets.emplace_back(unique_ptr<UDPSocket>(new UDPSocket(addr, port, so_reuse)));
}

///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<UDPSocket> UDPListener::takeSocket(UDPSocket* socket) {
   for (auto it = _sockets.begin(); it != _sockets.end(); it++){
      //compare memory addresses
      if (socket == it->get()){
         //efficient removal, swap with back and pop
         std::swap(*it, _sockets.back());
         auto removed = std::move(_sockets.back());
         _sockets.pop_back();
         return removed;
      }
   }
   //no socket to remove
   return {};
}

///////////////////////////////////////////////////////////////////////////////
UDPSocket* UDPListener::getNextReady(std::chrono::milliseconds timeout) {
   for (auto& socket : _sockets) {
      fd_set readfds;

      FD_ZERO(&readfds);
      FD_SET(socket->sockfd, &readfds);

      timeval timeout_tv;
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
      auto remaining_ms = timeout - seconds;

      timeout_tv.tv_sec = static_cast<decltype(timeout_tv.tv_usec)>(seconds.count());
      timeout_tv.tv_usec = static_cast<decltype(timeout_tv.tv_usec)>(std::chrono::duration_cast<std::chrono::microseconds>(remaining_ms).count());

      // Wait for activity on any of the file descriptors
      int activity = select(static_cast<int>(socket->sockfd), &readfds, nullptr, nullptr, &timeout_tv);

      if (activity < 0) {
         std::cerr << "UDPListener:: Select error: " << strerror(errno) << std::endl;
         break;
      } else if (activity == 0) {
         // Timeout occurred
         continue;
      }

      // Check if there's data on the UDP socket
      if (FD_ISSET(socket->sockfd, &readfds)) return socket.get();
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UDPSender::UDPSender(const std::string& addr, uint32_t port, bool so_reuse){
   socket = unique_ptr<UDPSocket>(new UDPSocket("0.0.0.0", 0, true));
   socket->setSendAddr(addr, port);
}

///////////////////////////////////////////////////////////////////////////////
void UDPSender::send(char *buf, size_t size) {
   socket->send(buf, size);
}

///////////////////////////////////////////////////////////////////////////////
void UDPSender::send(const std::string& str) {
   socket->send(str.c_str(), str.size());
}

///////////////////////////////////////////////////////////////////////////////
void UDPSender::bindLocalAddr(const std::string& addr, uint32_t port) {
   socket = unique_ptr<UDPSocket>(new UDPSocket(addr, port, true));
}