#pragma once
/* The few socket calls TestServer needs, for POSIX and Winsock. Plain C, and kept in its own file, so the OS headers
 * (windows.h above all, whose names clash with raylib's) never meet the engine's. Blocking calls take a timeout, so the
 * socket thread can keep checking whether it should stop. */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t ReyTestSocket;
#define REY_TEST_SOCKET_INVALID ((ReyTestSocket)-1)

/* start the socket library (Winsock needs it; elsewhere it does nothing). Returns 0 on success. Pair each successful
 * call with reyTestSocketCleanup. */
int reyTestSocketInit(void);
void reyTestSocketCleanup(void);
/* a socket listening on 127.0.0.1:port, or REY_TEST_SOCKET_INVALID */
ReyTestSocket reyTestSocketListen(uint16_t port);
/* the next client to connect, or REY_TEST_SOCKET_INVALID if none did within timeoutMs */
ReyTestSocket reyTestSocketAccept(ReyTestSocket listener, int timeoutMs);
/* read what has arrived, waiting up to timeoutMs: the number of bytes read, 0 if nothing came, or -1 if the client
 * closed the connection or it failed */
int reyTestSocketRecv(ReyTestSocket client, char* buffer, int size, int timeoutMs);
/* send it all, or as much as the client will take before it goes. Never raises SIGPIPE. */
void reyTestSocketSend(ReyTestSocket client, const char* data, int size);
void reyTestSocketClose(ReyTestSocket socket);

#ifdef __cplusplus
}
#endif
