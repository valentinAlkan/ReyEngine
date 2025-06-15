#include "Network.h"
#include "StringTools.h"
#include <iostream>
#include "StringTools.h"
#ifdef PLATFORM_WINDOWS
#include <WS2tcpip.h>
#elif defined(PLATFORM_LINUX)
#include <cstring> //for strerror
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