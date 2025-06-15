#pragma once
#include "Network.h"
#include <optional>
#include <chrono>
#include <sys/types.h>
#include <stdexcept>
#include <cstring>
#include <vector>

struct addrinfo;
struct sockaddr_in;
namespace Sockets {

   class UDPRuntimeError : public std::runtime_error {
   public:
      UDPRuntimeError(const char *w) : std::runtime_error(w) {}
   };

   struct HostInfo{
      std::string ip;
      int port;
      friend std::ostream& operator<<(std::ostream& os, const HostInfo& host) {return os << host.ip << ":" << host.port;}
   };

   class UDPListener;
   class UDPSocket{
      //addrinfo helper
      struct AddressInfo{
         ~AddressInfo();
         AddressInfo(const std::string& addr, uint32_t port);
         int                 _port;
         std::string         _strAddr;
         std::string         _strPort;
         addrinfo*           _addrinfo;
         char                _decimal_port[16];
      };
   public:
      ~UDPSocket();
      /// Two sockets are the same if they have the same socket descriptor
      bool operator==(const UDPSocket& other) const {return sockfd == other.sockfd;}
      void send(const char* buf, size_t size);
      int recv(char *recvBuf, size_t maxSize);
      [[nodiscard]] HostInfo getRecvAddr() const;
   protected:
      UDPSocket(const std::string& addr, uint32_t port, bool so_reuse);
      [[nodiscard]] int                 getPort() const;
      [[nodiscard]] std::string         getAddr() const;
      std::string         parseClientIP(sockaddr_in* srcaddr) const;
      int                 parseClientPort(sockaddr_in* srcaddr) const;
      void                setSendAddr(const std::string& host, uint32_t port);
      //optional port means let the system pick - best used for senders
      void                bind(const std::string& addr, uint32_t port, bool reuseAddr);
      SOCKFD              sockfd; //socket descriptor
      std::unique_ptr<AddressInfo>         bindAddr;
      std::unique_ptr<AddressInfo>         sendAddr;
      bool _isValid       = false;
      //address of last host to send us data. pointer because we cant include winsock in a header file
      sockaddr_in* last_srcaddr = nullptr;
      friend class UDPListener;
      friend class UDPSender;
   };

   class UDPListener{
   public:
      void listen(const std::string& addr, uint32_t port, bool so_reuse=true);
      UDPSocket* getNextReady(std::chrono::milliseconds timeout);
   private:
      std::unique_ptr<UDPSocket> takeSocket(UDPSocket*);
      std::vector<std::unique_ptr<UDPSocket>> _sockets;
   };

   class UDPSender{
   public:
      UDPSender(const std::string& addr, uint32_t port, bool so_reuse=true);
      void send(char* buf, size_t size);
      void send(const std::string& str);
      void bindLocalAddr(const std::string& addr, uint32_t port);
   private:
      std::unique_ptr<UDPSocket> socket;
   };
}