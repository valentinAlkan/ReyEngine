#pragma once
#include "Network.h"
#include <optional>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <vector>

namespace Sockets {

   class UDPRuntimeError : public std::runtime_error {
   public:
      UDPRuntimeError(const char *w) : std::runtime_error(w) {}
   };

   class UDPListener;
   class UDPSocket{
   public:
      ~UDPSocket();
      /// Two sockets are the same if they have the same socket descriptor
      bool operator==(const UDPSocket& other) const {return sockfd == other.sockfd;}
      void send(const char* buf, size_t size);
      int recv(char *recvBuf, size_t maxSize);
      [[nodiscard]] HostInfo getRecvAddr() const;
      [[nodiscard]] HostInfo getBindAddr() const;
   protected:
      UDPSocket(const std::string& addr, uint32_t port, bool so_reuse);
      [[nodiscard]] int                 getPort() const;
      [[nodiscard]] std::string         getAddr() const;
      void                setSendAddr(const std::string& host, uint32_t port);
      //optional port means let the system pick - best used for senders
      void                bind(const std::string& addr, uint32_t port, bool reuseAddr);
      SOCKFD              sockfd; //socket descriptor
      std::unique_ptr<AddressInfo>         bindAddr;
      std::unique_ptr<AddressInfo>         sendAddr;
      bool _isValid       = false;
      //address of last host to send us data. pointer because we cant include winsock in a header file
      sockaddr_in* last_recvaddr = nullptr;
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