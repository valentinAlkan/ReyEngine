#include "TCP.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>
#include "Platform.h"
#include "StringTools.h"
#include "Logger.h"

#ifdef PLATFORM_WINDOWS
#include "Network.h"
#include <ws2tcpip.h>
#include <memory>
#endif

using namespace std;
using namespace chrono;
using namespace ReyEngine;

//do not put in header file, addrinfo cannot appear in a header file in windows
namespace TCP{
    //automatically frees any memory used in getAddrInfo
    struct AutoAddrInfo{
        AutoAddrInfo(){
            info = nullptr;
        }
        ~AutoAddrInfo(){
            freeaddrinfo(info);
        }
        addrinfo* info;
    };
}


#ifdef PLATFORM_WINDOWS
void TCPClient::printNetworkError() {
   NetworkError e;
   stringstream ss;
   ss << "<" << ip << ":"<< port << ">: " << e.err << ":  " << e.msg;
   logMsg(ss.str(), LogLevel::ERR);
}
#endif


///////////////////////////////////////////////////////////////////////////////
bool TCPClient::connect(){
    scoped_lock<std::mutex> sl(_lock);
    if (_doCleanup) {
        //no connects possible once cleanup invoked
        return false;
    }
#ifdef PLATFORM_WINDOWS
    TCP::AutoAddrInfo result;
    addrinfo* ptr = nullptr; //do not free
    addrinfo hints = {}; //do not free
    int iResult;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result.info);
    if ( iResult != 0 ) {
        printNetworkError();
        WinNet::WinSockInit::instance();
        return false;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result.info; ptr != nullptr ;ptr=ptr->ai_next) {
        //make sure we're not trying to connect to 0.0.0.0, which will not work and log spurious errors
        if (ptr->ai_addr->sa_data[0] == 0 &&
            ptr->ai_addr->sa_data[1] == 0 &&
            ptr->ai_addr->sa_data[3] == 0 &&
            ptr->ai_addr->sa_data[4] == 0) continue;
        // Create a SOCKET for connecting to server
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            printNetworkError();
            return false;
        }

        // Set socket to non-blocking
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);

        // Connect to server.
        int result = ::connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sock, &writefds);

            struct timeval tv;
            tv.tv_sec = 1; //one sec timeout
            tv.tv_usec = 0;

            result = select(sock + 1, nullptr, &writefds, nullptr, &tv);
            if (result <= 0) { // Timeout or error
                if (result) printNetworkError();
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }

            // Set socket back to blocking mode
            mode = 0;
            ioctlsocket(sock, FIONBIO, &mode);
        }
        break;
    }

    if (sock == INVALID_SOCKET) {
        Logger::warn() << "TCP: Unable to connect to server " << ip << ":" << to_string(port) << endl;
        return false;
    }
#elif defined(PLATFORM_LINUX)
    sockaddr_in serv_addr = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("TCPClient: Unable to create TCP Socket\n");
        close(sock);
        sock = -1;
        return false;
    }
   
    char enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        perror("TCPClient: setsockopt(SO_REUSEADDR) failed");
    }

    
    //1 - sec
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0){
        printf("TCPClient: setsockopt(SO_SNDTIMEO) failed\n");
        close(sock);
        sock = -1;
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

   //convert hostname to ip
   char addr_buf[64] = {};
   TCP::AutoAddrInfo feed_server;
   addrinfo* f_addrinfo = nullptr;
   getaddrinfo(ip.c_str(), nullptr, nullptr, &feed_server.info);
   for(f_addrinfo = feed_server.info; f_addrinfo != nullptr; f_addrinfo = f_addrinfo->ai_next){
      if ( f_addrinfo->ai_family == AF_INET){
         inet_ntop(AF_INET, &((sockaddr_in *)f_addrinfo->ai_addr)->sin_addr, addr_buf, sizeof(addr_buf));
      }
   }

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, addr_buf, &serv_addr.sin_addr) <= 0) {
        printf("TCPClient: Invalid address/ Address not supported\n");
        close(sock);
        sock = -1;
        return false;
    }
   
    if (::connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        Logger::warn() << "TCPClient: Connection to host " << ip << " failed" << std::endl;
        close(sock);
        sock = -1;
        return false;
    }
#endif
    _ready = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
int TCPClient::send(const char* msg, size_t len){
    if (!isConnected()){
       cout << "TCPClient: not connected to endpoint " << ip << ":" << port;
       return -1;
    }
    #ifdef PLATFORM_WINDOWS
    int nbytes = ::send(sock, msg, len, 0);
    #elif defined(PLATFORM_LINUX)
    int nbytes = ::send(sock, msg, len, MSG_NOSIGNAL);
    #endif
    if (nbytes < 0){
        disconnect();
        int errorCode = errno;
#ifdef PLATFORM_WINDOWS
        errorCode = WSAGetLastError();
#endif
        printf("TCPClient: send failure %d\n", errorCode);
    }
    return nbytes;
}

///////////////////////////////////////////////////////////////////////////////
int TCPClient::recv(char* msg, size_t len, chrono::milliseconds timeout){
    if (!isConnected()){
        printf("TCPClient: not connected\n");
        return -1;
    }
    scoped_lock<std::mutex> sl(_lock);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    if (timeout.count()) {
        timeval tv;
        tv.tv_sec = timeout.count() / 1000;
        tv.tv_usec = timeout.count() % 1000;

        auto result = select(sock + 1, &fds, 0, 0, &tv);
        if (result == 0) {
            return 0;
        } else if (result < 0) {
            perror("TCPClient: select error: ");
            return -1;
        }
    }

#ifdef PLATFORM_WINDOWS
    int nbytes = ::recv(sock, msg, len, 0);
#elif defined(PLATFORM_LINUX)
    int nbytes = ::recv(sock, msg, len, MSG_NOSIGNAL);
#endif
    if (nbytes < 0){
        int errorCode = errno;
#ifdef PLATFORM_WINDOWS
        errorCode = WSAGetLastError();
#endif
        if (errorCode != 11) {
            printf("TCPClient: recv failure %d (%s)\n", errorCode, ::strerror(errorCode));
        }
    }
    return nbytes;
}
///////////////////////////////////////////////////////////////////////////////
bool TCPClient::disconnect(){
    scoped_lock<std::mutex> sl(_lock);
    _ready = false;
    if (sock) close(sock);
    sock = -1;
    return _ready;
}
   
///////////////////////////////////////////////////////////////////////////////
bool TCPClient::isConnected() const {
    return _ready;
}

///////////////////////////////////////////////////////////////////////////////
void TCPClient::logMsg(const std::string& msg, LogLevel level){
   string_tools::rstrip(msg);
   if (level >= logLevel){
        std::cout << "TcpClient: " << msg << endl;
    }
    std::scoped_lock<std::mutex> sl(messageLock);
    //if the log is too big, pop the oldest message
    if (messageLog.size() > LOG_SIZE_MAX){
        messageLog.pop();
    }
    messageLog.emplace(msg, level);
}
///////////////////////////////////////////////////////////////////////////////
TCPClient::LogMsg TCPClient::getLogMsg() {
    LogMsg retval;
    std::scoped_lock<std::mutex> sl(messageLock);
    retval = messageLog.front();
    messageLog.pop();
    return retval;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    #ifdef PLATFORM_WINDOWS
    //! not sure what this should do
    //! while(WaitForSingleObject(-1, NULL));
    #elif defined(PLATFORM_LINUX)
    while(waitpid(-1, NULL, WNOHANG) > 0);
    #endif

    errno = saved_errno;
}

///////////////////////////////////////////////////////////////////////////////
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

///////////////////////////////////////////////////////////////////////////////
void TCPServer::start(){
    runThread = std::thread(&TCPServer::run, this);
}

///////////////////////////////////////////////////////////////////////////////
void TCPServer::stop(){
    kill = true;
    cout << "TCPServer : TCPServer got kill signal, waiting to die" << endl;
    if (runThread.joinable()) {
        runThread.join();
    }
}

///////////////////////////////////////////////////////////////////////////////
int TCPServer::run(){
#define BACKLOG 10   // how many pending connections queue will hold
#define DEFAULT_BUFLEN 512
    int retval;
    struct ServerData {
        ServerData(){
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;
        }
        ~ServerData(){
#ifdef PLATFORM_WINDOWS
            if (listener_fd) {
                closesocket(listener_fd);
            }
#elif defined(PLATFORM_LINUX)
            if (listener_fd) {
                ::close(listener_fd);
            }
#endif
        }
        SOCKFD listener_fd, client_fd; //listener listens for connections, client_fd is client connection
        addrinfo hints, *p;
        TCP::AutoAddrInfo servinfo;
        sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
        int iSendResult;
        char yes=1;
        char s[INET6_ADDRSTRLEN];
        int rv;

    } data;

    //resolve hostnames
    char addr_buf[64];
    struct addrinfo* feed_server = nullptr;
    getaddrinfo(ip.c_str(), nullptr, nullptr, &feed_server);
    for(data.servinfo.info = feed_server; data.servinfo.info != nullptr; data.servinfo.info = data.servinfo.info->ai_next){
        if ( data.servinfo.info->ai_family == AF_INET){
            inet_ntop(AF_INET, &((struct sockaddr_in *)data.servinfo.info->ai_addr)->sin_addr, addr_buf, sizeof(addr_buf));
        }
    }

#ifdef PLATFORM_WINDOWS
   // Resolve the server address and port
   retval = getaddrinfo(addr_buf, std::to_string(port).c_str(), &data.hints, &data.servinfo.info);
   if ( retval != 0 ) {
      printf("TCPServer: getaddrinfo failed with error: %d\n", retval);
      return 1;
   }

   // Create a SOCKET for the server to listen for client connections.
   data.listener_fd = socket(data.servinfo.info->ai_family, data.servinfo.info->ai_socktype, data.servinfo.info->ai_protocol);
   if (data.listener_fd == INVALID_SOCKET) {
      printf("TCPServer: socket failed with error: %ld\n", WSAGetLastError());
      return 1;
   }

   // Setup the TCP listening socket
   retval = bind(data.listener_fd, data.servinfo.info->ai_addr, (int)data.servinfo.info->ai_addrlen);
   if (retval == SOCKET_ERROR) {
      printf("TCPServer: bind failed with error: %d\n", WSAGetLastError());
      return 1;
   }

   retval = listen(data.listener_fd, SOMAXCONN);
   if (retval == SOCKET_ERROR) {
      printf("TCPServer: listen failed with error: %d\n", WSAGetLastError());
      return 1;
   }

#elif defined(PLATFORM_LINUX)
    if ((data.rv = getaddrinfo(NULL, (const char*)std::to_string(port).c_str(), &data.hints, &data.servinfo.info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(data.rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for (data.p = data.servinfo.info; data.p != NULL; data.p = data.p->ai_next) {
        if ((data.listener_fd = socket(data.p->ai_family, data.p->ai_socktype, data.p->ai_protocol)) == -1) {
            perror("server: socket");
            close(data.listener_fd);
            continue;
        }


        if (setsockopt(data.listener_fd, SOL_SOCKET, SO_REUSEADDR, &data.yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(data.listener_fd, data.p->ai_addr, data.p->ai_addrlen) == -1) {
            perror("server: bind");
            continue;
        }
        break;
    }

    if (data.p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(data.listener_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(1);
    }
#endif

    printf("TCPServer: waiting for connections...\n");

    unsigned long long connectionId = 0;
    fd_set read_fd_set;
    int nBytes;
    char tbuf[RECVBUF_SIZE];
    while (!kill) {
        governor.wait();
        FD_ZERO(&read_fd_set);
        /* Set the fd_set before passing it to the select call */
        if (data.listener_fd != -1) {
            FD_SET(data.listener_fd, &read_fd_set);
        }
        for (const auto& connection : _connections) {
            int fd = connection->getFd();
            if (fd != -1 && !connection->isDead()) {
                FD_SET(fd, &read_fd_set);
            }
        }

        /* Invoke select() and then wait! */
        struct timeval timeout {1,0};
        int selectRetval = select(FD_SETSIZE, &read_fd_set, nullptr, nullptr, &timeout);

        /* select() woke up. Identify the fd that has events */
        if (selectRetval >= 0) {
            if (data.listener_fd != -1 && FD_ISSET(data.listener_fd, &read_fd_set)) {
                data.sin_size = sizeof(data.their_addr);
                data.client_fd = accept(data.listener_fd, (struct sockaddr*)&data.their_addr, &data.sin_size);
                if (data.client_fd == -1) {
                    perror("TCPServer: accept");
                    continue;
                }
                inet_ntop(data.their_addr.ss_family, get_in_addr((struct sockaddr*)&data.their_addr), data.s, sizeof(data.s));
                cout << "TCPServer: got connection from " << data.s << " on port " << port << endl;
                {
                    std::scoped_lock<std::mutex> sl(mtexConnection);
                    _connections.emplace_back();
                    _connections.back().reset(new TCPStream(data.client_fd, string(data.s), connectionId++));
                    _newConnections.push(_connections.back()); //copy the connection to the new connections vector
                }
                data.client_fd = -1;
            } else {
                for (const auto& connection : _connections) {
                    int fd = connection->getFd();
                    if (!connection->isDead() && fd != -1 && FD_ISSET(fd, &read_fd_set)) {
                        /* read incoming data */
                        nBytes = recv(fd, tbuf, RECVBUF_SIZE, 0);
                        switch (nBytes) {
                            case -1:
                                perror("recv");
                                connection->hangUp();
                                break;
                            case 0:
                                connection->hangUp(); //garbage collector will take care of it
                                break;
                            default:
                                connection->write(tbuf, nBytes);
                                break;
                        }
                    }
                }
            }
        } else {
            if (errno) {
                cerr << "Select FD err (" << errno << "): " << strerror(errno) << endl;
            }
        }
        collectGarbage();
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
std::optional<std::weak_ptr<TCPStream>> TCPServer::getNewConnection() {
    if (_newConnections.empty()) return nullopt;
    auto top = _newConnections.top();
    _newConnections.pop();
    return top;
}

///////////////////////////////////////////////////////////////////////////////
void TCPServer::collectGarbage(){
    std::scoped_lock<std::mutex> sl(mtexConnection);
    for (auto it=_connections.begin(); it != _connections.end(); /**/){
        auto& connection = *it;
        //if the connection needs to be killed, or has died, delete it
        if(connection->isDead()){
            cout << "TCPServer: Client " << connection->getId() << " has died, deleting conection" << endl;
            it = _connections.erase(it);
        } else {
            ++it;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TCPStream::~TCPStream() {
    kill = true;
    close(fd);
}

///////////////////////////////////////////////////////////////////////////////
bool TCPStream::isDead() const {
    return kill || chrono::steady_clock::now() - _lastSeen > _keepAlive;
}

///////////////////////////////////////////////////////////////////////////////
void TCPStream::write(char* buf, int nBytes){
    _lastSeen = steady_clock::now();
    circBuf.write(buf, nBytes);
}

///////////////////////////////////////////////////////////////////////////////
int TCPStream::read(char* buf, size_t maxBytes){
    return circBuf.read(buf, maxBytes);
}

///////////////////////////////////////////////////////////////////////////////
int TCPStream::available(){
    return circBuf.available();
}

///////////////////////////////////////////////////////////////////////////////
char TCPStream::peek(size_t i) const {
    return circBuf.peek(i);
}

///////////////////////////////////////////////////////////////////////////////
void TCPStream::send(string stdStringMessage){
    send(stdStringMessage.c_str(), stdStringMessage.size());
}
///////////////////////////////////////////////////////////////////////////////
void TCPStream::send(const char* message, int nBytes){
    #ifdef PLATFORM_WINDOWS
    if (::send(fd, message, nBytes, 0) == -1){
    #elif defined(PLATFORM_LINUX)
    if (::send(fd, message, nBytes, MSG_NOSIGNAL) == -1){
    #endif
        printf("ConnectionError: closing TCPServerConnection %lld to remote host %s\n ", id, ip.c_str());
        kill = true;
    } else {
        _lastSeen = steady_clock::now();
    }
}