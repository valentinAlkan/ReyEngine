#pragma once
#include "Network.h"
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>
#include <optional>
#include <stack>
#include <queue>

#include "CircBuf.h"
#include "SystemTime.h"

class TCPDevice {
protected:
    TCPDevice(std::string ip, int port): ip(ip), port(port), governor(120){}
    std::string ip;
    int port;
protected:
    ReyEngine::Time::RateLimiter governor;
public:
    const std::string& getIp(){return ip;}
    int getPort(){return port;}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class TCPClient : public TCPDevice {
public:
    static constexpr size_t LOG_SIZE_MAX = 1000;
    enum class LogLevel {DEBUG, INFO, WARN, ERR};
    using LogMsg = std::pair<std::string, LogLevel>;
    TCPClient(std::string ip, int port)
    : TCPDevice(ip, port)
    {
#ifdef PLATFORM_WINDOWS
      WinNet::WinSockInit::instance();
#endif
    }
    ~TCPClient(){
        std::scoped_lock<std::mutex> sl(_lock);
        _doCleanup = true;
    }
    int send(const char*, size_t);
    int recv(char*, size_t, std::chrono::milliseconds);
    bool isReady() const {return _ready;}
    bool connect();
    bool disconnect();
    bool isConnected() const;
    LogMsg getLogMsg();
    LogLevel getLogLevel(){return logLevel;}
private:
    void printNetworkError();
    LogLevel logLevel = LogLevel::DEBUG; //print all messages by default
    void logMsg(const std::string&, LogLevel);
    std::queue<LogMsg> messageLog;
    std::mutex messageLock;
    std::mutex _lock;
    bool _ready = false;
    bool _doCleanup = false;
    SOCKFD sock = -1;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define CONNECTION_BUFFER_SIZE 1024
class TCPStream{
public:
    ~TCPStream();
    std::string getIP(){return ip;}
    [[nodiscard]] unsigned long long getId() const {return id;}
    void send(const char* message, int nBytes);
    void send(std::string stdStringMessage);
    int available();
    [[nodiscard]] char peek(size_t i) const ;
    int read(char* buf, size_t maxBytes);
    CircularBuffer& data(){return circBuf;}
    void hangUp(){
        std::scoped_lock<std::mutex> lock{mtx};
        kill = true;
    }
    void setKeepAlive(std::chrono::minutes mins){_keepAlive = mins;}
    SOCKFD getFd() const {return fd;}
    bool isDead() const;
protected:
    TCPStream(int fd, const std::string& ip, unsigned int id, int circBufSize = 1024, std::chrono::minutes keepAlive = std::chrono::minutes(60))
    : fd(fd)
    , ip(ip)
    , id(id)
    , circBuf(circBufSize)
    , _keepAlive(keepAlive)
    , _lastSeen(std::chrono::steady_clock::now())
    {}
    void write(char* buf, int nBytes);
private:
    SOCKFD fd;
    std::chrono::minutes _keepAlive; //how long the connection should live without receiving or sending data
    std::chrono::steady_clock::time_point _lastSeen; //the last time we sent or recieved data
    std::string ip;
    unsigned long long id; //just any old id number to use
    bool kill = false; //the thread needs to be killed
    CircularBuffer circBuf;
    std::mutex mtx; //used to make sure nobody is reading it when we delete it
    friend class TCPServer;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MAX_CONNECTIONS 256
#define RECVBUF_SIZE 256

class TCPServer : public TCPDevice {
public:
    TCPServer(int port)
     : TCPDevice("0.0.0.0", port)
     {
#ifdef PLATFORM_WINDOWS
        WinNet::WinSockInit::instance();
#endif
    }
    ~TCPServer(){
        stop();
        if (runThread.joinable()) {
            runThread.join();
        }
    }
    void start();
    void stop();
    void send(std::shared_ptr<TCPStream> connection, char* bytes, int byteCount);
    std::vector<std::shared_ptr<TCPStream>> getConnections(){return _connections;} //copies vector. no bueno?
    std::optional<std::weak_ptr<TCPStream>> getNewConnection();
private:
    std::vector<std::shared_ptr<TCPStream>> _connections;
    std::stack<std::weak_ptr<TCPStream>> _newConnections;
    int run();
    void collectGarbage();
    std::thread runThread;
    bool kill = false;
    std::mutex mtexConnection;
};