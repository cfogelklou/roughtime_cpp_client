#include "mini_socket.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
typedef int SOCKET;
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define GetLastError() (-1)
#endif

#include <cstdio>
#include <cstring>
#include <queue>
#include <thread>
#include <mutex>          // std::mutex, std::unique_lock, std::defer_lock

#include "cs_task_locker.hpp"
#include "byte_q.hpp"
#include "queue_base.hpp"


#ifdef WIN32
#define OSALSleep(x) Sleep(x)
#else
#define OSALSleep(x) usleep((uint64_t)(x)*1000ull)
#define WSAGetLastError() errno
#endif

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define LOG_TRACE(x) std::printf x
#define LOG_WARNING(x) std::printf x
#define LOG_ASSERT_WARN(state) if (!(state)) \
do { LOG_WARNING(("Assertion Failed at %s(%d)\r\n", __FILE__, __LINE__)); } while(0)

// ////////////////////////////////////////////////////////////////////////////
class SerSocket {
public:
  static SerSocket &inst() {
    static SerSocket local;
    return local;
  }

  void Init() {}

  // Constructor.  Init sockets.
  SerSocket() {
#ifdef _WIN32
    static WSADATA wsa_data;
    WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif
  }

  // Destructor, deinit sockets.
  ~SerSocket() {
#ifdef _WIN32
    WSACleanup();
#endif
  }

  // Close a socket.
  int sockClose(const SOCKET sock) {
    if (sock == INVALID_SOCKET) return 0;
    int status = 0;
#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) { status = closesocket(sock); }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { status = close(sock); }
#endif
    return status;
  }
};

#define BUFLEN 2048
class UdpClient: public QueueBase {
public:

public:
  UdpClient(const char *szAddr, const int port)
  : QueueBase()
  , mSocket(INVALID_SOCKET)
  , mRxQ()
  {
    SerSocket::inst().Init();
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_INET; // AF_INIT == internetwork: UDP, TCP, etc.
    hints.ai_socktype   = SOCK_DGRAM; /* datagram socket */
    hints.ai_protocol   = IPPROTO_UDP;

    hints.ai_flags |= AI_ALL; // Query both IP6 and IP4 with AI_V4MAPPED

    // Resolve the server address and port
    char szPort[10];
    snprintf(szPort, sizeof(szPort), "%d", port);
    int result = getaddrinfo(szAddr, szPort, &hints, &mpAddrInfo);
    if (result != 0) {
      LOG_TRACE(("getaddrinfo: %s\n", gai_strerror(result)));
      exit(EXIT_FAILURE);
    }
        
    //Create a socket
    if((mSocket = socket(mpAddrInfo->ai_family, mpAddrInfo->ai_socktype, mpAddrInfo->ai_protocol )) == INVALID_SOCKET){
      auto err = WSAGetLastError();
      LOG_TRACE(("socket: %s\n", gai_strerror(err)));
      exit(-1);
    }
  }

  // //////////////////////////////////////////////////////////////////////////
  ~UdpClient(){
    freeaddrinfo(mpAddrInfo);    
    SerSocket::inst().sockClose(mSocket);
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t Write(const uint8_t arr[], const size_t len) override {
    bool ok = true;
    int stat = sendto(mSocket, (const char *)arr, len, 0, mpAddrInfo->ai_addr, mpAddrInfo->ai_addrlen);
    ok = (stat == len);
    if (!ok) {
      auto err = WSAGetLastError();
      LOG_TRACE(("send: %s\n", gai_strerror(err)));
      exit(EXIT_FAILURE);
    }

#ifdef WIN32
    if (ok) {
      static int timeout = 1000;
      stat = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
      ok = (stat != SOCKET_ERROR);
    }
#endif

    if (ok){
      // Use thread to read from socket to prevent blocking.
      auto ReadFromSocketThread = [](void *pThis) {
        UdpClient &th = *(UdpClient *)pThis;

        char  rxBuf[BUFLEN] = {0};
        socklen_t alen = th.mpAddrInfo->ai_addrlen;
        const int rx = recvfrom(th.mSocket, rxBuf, sizeof(rxBuf), 0, th.mpAddrInfo->ai_addr, &alen);
        if (SOCKET_ERROR != rx) {
          th.mRxQ.Write((uint8_t *)rxBuf, rx);
        }
        else {
          const int err = WSAGetLastError();
          LOG_TRACE(("recv: %s\n", gai_strerror(err)));
        }
      };

      std::thread t(ReadFromSocketThread, this);
      t.detach();
    }
    return (ok) ? len : 0;
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t Read(uint8_t arr[], const size_t len) override {
    return mRxQ.Read(arr, len);
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t GetWriteReady() override {
    return BUFLEN;
  }
  
  // //////////////////////////////////////////////////////////////////////////
  size_t GetReadReady() override {
    return mRxQ.GetReadReady();
  }

private:
  struct addrinfo  *mpAddrInfo;
  SOCKET            mSocket;
  ByteQ             mRxQ;

};

// ////////////////////////////////////////////////////////////////////////////
QueueBase &CreateUdpClient(const char *szAddr, const int port){
  auto p = new UdpClient(szAddr, port);
  return *p;
}

// ////////////////////////////////////////////////////////////////////////////
void DeleteUdpClient(QueueBase *p){
  UdpClient *ps = (UdpClient *)p;
  delete ps;
}
