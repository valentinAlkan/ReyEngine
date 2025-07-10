#include "Network.h"
#include "UDP.h"
#include "Logger.h"
#include <iostream>

#ifdef PLATFORM_WINDOWS
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#endif

using namespace std;
using namespace Sockets;
using namespace ReyEngine;

////////////////////////////////////////////////////////////////////////////////////////
// Helper function to resolve hostname to IP address
std::string resolveHostname(const std::string& hostname) {
   if (hostname == "0.0.0.0" || hostname.empty()) {
      return "0.0.0.0";  // INADDR_ANY case
   }

   if (hostname == "localhost") {
      return "127.0.0.1";
   }

   // Check if it's already an IP address
   struct sockaddr_in sa;
   if (inet_pton(AF_INET, hostname.c_str(), &(sa.sin_addr)) == 1) {
      return hostname;  // Already an IP address
   }

   // Resolve hostname using getaddrinfo
   struct addrinfo hints, *result;
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;        // IPv4
   hints.ai_socktype = SOCK_DGRAM;   // UDP
   hints.ai_protocol = IPPROTO_UDP;

   int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
   if (status != 0) {
      throw UDPRuntimeError(("hostname resolution failed for: \"" + hostname + "\", error: " + gai_strerror(status)).c_str());
   }

   // Extract IP address from first result
   struct sockaddr_in* addr_in = (struct sockaddr_in*)result->ai_addr;
   char ip_str[INET_ADDRSTRLEN];
   inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);

   freeaddrinfo(result);
   return std::string(ip_str);
}

////////////////////////////////////////////////////////////////////////////////////////
UDPSocket::UDPSocket(const std::string& addr, uint32_t port, bool so_reuse){
   last_recvaddr = new sockaddr_in;
   bind(addr, port, so_reuse);
}

////////////////////////////////////////////////////////////////////////////////////////
UDPSocket::~UDPSocket(){
   if (_isValid) {
      close(sockfd);
   }
   delete last_recvaddr;
}

////////////////////////////////////////////////////////////////////////////////////////
void UDPSocket::bind(const std::string& addr, uint32_t port, bool reuseAddr){
#ifdef PLATFORM_WINDOWS
   WinNet::WinSockInit::instance();
#endif

   // Resolve hostname to IP address
   std::string resolved_addr;
   try {
      resolved_addr = resolveHostname(addr);
   } catch (const UDPRuntimeError& e) {
      throw UDPRuntimeError(("could not resolve hostname for binding: \"" + addr + "\", error: " + e.what()).c_str());
   }


   // Create socket
   sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if(sockfd == -1) {
      NetworkError e;
      Logger::error() << "UDPSocket socket() creation failed, errno " << e.err << " : " << e.msg << endl;
      throw UDPRuntimeError(("could not create UDP socket for: \"" + resolved_addr + "\", error: " + e.msg).c_str());
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
   if (resolved_addr == "0.0.0.0" || resolved_addr.empty()) {
      bind_addr.sin_addr.s_addr = INADDR_ANY;
   } else if (resolved_addr == "localhost") {
      bind_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   } else {
      if (inet_pton(AF_INET, resolved_addr.c_str(), &bind_addr.sin_addr) != 1) {
         close(sockfd);
         throw UDPRuntimeError(("invalid address: \"" + resolved_addr + "\"").c_str());
      }
   }

   // Bind socket
   if (::bind(sockfd, (sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
      NetworkError e;
      Logger::error() << "UDPSocket Bind errno " << e.err << " : " << e.msg << endl;
      close(sockfd);
      throw UDPRuntimeError(("could not bind UDP socket with: \"" + resolved_addr + ":" + std::to_string(port) + "\"").c_str());
   }

   // Get the actual bound address and port (important for port 0)
   socklen_t addr_len = sizeof(bind_addr);
   if (getsockname(sockfd, (sockaddr*)&bind_addr, &addr_len) == 0) {
      uint32_t actual_port = ntohs(bind_addr.sin_port);

      // Create AddressInfo with the actual bound port
      bindAddr = make_unique<AddressInfo>(resolved_addr, actual_port);

      if (port == 0) {
         Logger::info() << "UDPSocket: Port 0 was assigned to " << actual_port << " by the system" << endl;
      }
   } else {
      // Fallback: create AddressInfo with requested port
      bindAddr = make_unique<AddressInfo>(resolved_addr, port);
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
   return ::recvfrom(sockfd, recvBuf, (int)max_size, 0, (sockaddr*)last_recvaddr, &addrlen);
}

////////////////////////////////////////////////////////////////////////////////////////
 HostInfo UDPSocket::getRecvAddr() const {
     return {last_recvaddr};
 }

////////////////////////////////////////////////////////////////////////////////////////
HostInfo UDPSocket::getBindAddr() const {
   return {bindAddr->_strAddr, bindAddr->_port};
}

////////////////////////////////////////////////////////////////////////////////////////
void UDPSocket::setSendAddr(const std::string& host, uint32_t port) {
   sendAddr = make_unique<AddressInfo>(host, port);
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
   fd_set readfds;
   FD_ZERO(&readfds);

   SOCKFD max_fd = 0;
   for (auto& socket : _sockets) {
      SOCKFD fd = socket->sockfd;
      FD_SET(fd, &readfds);
      max_fd = max(max_fd, fd);
   }

   timeval timeout_tv;
   auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
   auto remaining_ms = timeout - seconds;

   timeout_tv.tv_sec = static_cast<decltype(timeout_tv.tv_usec)>(seconds.count());
   timeout_tv.tv_usec = static_cast<decltype(timeout_tv.tv_usec)>(std::chrono::duration_cast<std::chrono::microseconds>(remaining_ms).count());

   // Wait for activity on any of the file descriptors
   int ready = select(static_cast<int>(max_fd) + 1, &readfds, nullptr, nullptr, &timeout_tv);

   if (ready < 0) {
      NetworkError err;
      std::cerr << "UDPListener:: Select error: " << strerror(errno) << std::endl;
   } else if (ready == 0) {
      //no sockets ready
      return {};
   } else {
      // a socket has data
      // Check which sockets are ready
      for (auto& socket : _sockets) {
         if (FD_ISSET(socket->sockfd, &readfds)) {
            return socket.get();
         }
      }
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UDPSender::UDPSender(const std::string& addr, uint32_t port, bool so_reuse)
: UDPSender(addr, port, "0.0.0.0", 0, so_reuse)
{}

///////////////////////////////////////////////////////////////////////////////
UDPSender::UDPSender(const std::string& addr, uint32_t port, uint32_t localPort, bool so_reuse)
: UDPSender(addr, port, "0.0.0.0", localPort, so_reuse)
{}

///////////////////////////////////////////////////////////////////////////////
UDPSender::UDPSender(const std::string& addr, uint32_t port, const std::string& localAddr, uint32_t localPort, bool so_reuse)
{
   socket = unique_ptr<UDPSocket>(new UDPSocket(localAddr, localPort, so_reuse));
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