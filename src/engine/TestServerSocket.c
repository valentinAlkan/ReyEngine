#include "TestServerSocket.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET OsSocket;
#define OS_INVALID INVALID_SOCKET
#define closeOsSocket closesocket
#define pollOs WSAPoll
#define SEND_FLAGS 0 /* Winsock never raises SIGPIPE */
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int OsSocket;
#define OS_INVALID (-1)
#define closeOsSocket close
#define pollOs poll
#define SEND_FLAGS MSG_NOSIGNAL /* a client that's gone must not kill the game */
#endif

/* SOCKET is unsigned on Windows and INVALID_SOCKET is all ones, so it round-trips through intptr_t as -1, the same as
 * a failed POSIX call */
static ReyTestSocket wrap(OsSocket s){return s == OS_INVALID ? REY_TEST_SOCKET_INVALID : (ReyTestSocket)s;}
static OsSocket unwrap(ReyTestSocket s){return (OsSocket)s;}

/* whether the socket has something to read (or has closed) within timeoutMs */
static int readable(ReyTestSocket s, int timeoutMs){
   struct pollfd fd;
   fd.fd = unwrap(s);
   fd.events = POLLIN;
   fd.revents = 0;
   return pollOs(&fd, 1, timeoutMs) > 0;
}

int reyTestSocketInit(void){
#ifdef _WIN32
   WSADATA data;
   return WSAStartup(MAKEWORD(2, 2), &data);
#else
   return 0;
#endif
}

void reyTestSocketCleanup(void){
#ifdef _WIN32
   WSACleanup();
#endif
}

ReyTestSocket reyTestSocketListen(uint16_t port){
   OsSocket s = socket(AF_INET, SOCK_STREAM, 0);
   if (s == OS_INVALID) return REY_TEST_SOCKET_INVALID;
#ifndef _WIN32
   /* so a restarted game can listen again straight away. Windows allows that anyway, and there SO_REUSEADDR would let
    * another program take over the port. */
   int yes = 1;
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
   struct sockaddr_in address = {0};
   address.sin_family = AF_INET;
   address.sin_port = htons(port);
   address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* local only */
   if (bind(s, (struct sockaddr*)&address, sizeof(address)) != 0 || listen(s, 1) != 0) {
      closeOsSocket(s);
      return REY_TEST_SOCKET_INVALID;
   }
   return wrap(s);
}

ReyTestSocket reyTestSocketAccept(ReyTestSocket listener, int timeoutMs){
   if (!readable(listener, timeoutMs)) return REY_TEST_SOCKET_INVALID;
   return wrap(accept(unwrap(listener), NULL, NULL));
}

int reyTestSocketRecv(ReyTestSocket client, char* buffer, int size, int timeoutMs){
   if (!readable(client, timeoutMs)) return 0;
   int got = (int)recv(unwrap(client), buffer, size, 0);
   return got > 0 ? got : -1;
}

void reyTestSocketSend(ReyTestSocket client, const char* data, int size){
   while (size > 0) {
      int sent = (int)send(unwrap(client), data, size, SEND_FLAGS);
      if (sent <= 0) return;
      data += sent;
      size -= sent;
   }
}

void reyTestSocketClose(ReyTestSocket socket){
   closeOsSocket(unwrap(socket));
}
