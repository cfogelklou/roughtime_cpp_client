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
#include <string.h>
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
#endif

static const int DEFAULT_BUFLEN = 2048;
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

// ////////////////////////////////////////////////////////////////////////////
class SerSocket {
public:
  static SerSocket &inst() {
    static SerSocket local;
    return local;
  }

  void Init() {
    //Nothing to do here...
  }

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

#define LOG_TRACE(x) std::printf x
#define LOG_WARNING(x) std::printf x
#define LOG_ASSERT_WARN(state) if (!(state)) \
do { LOG_WARNING(("Assertion Failed at %s(%d)\r\n", __FILE__, __LINE__)); } while(0)


class SerSocketQueue: public QueueBase {
public:
  
  
  const size_t BUFLEN = 2048;//  //Max length of buffer
  
  SerSocketQueue(const char *szAddr, const int port)
  : QueueBase()
  , mSocket(INVALID_SOCKET)
  , mRxByteQ()
  , mRxBuf{ 0 }
  {
    SerSocket::inst().Init();
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_INET; // AF_INIT == internetwork: UDP, TCP, etc.
    hints.ai_socktype   = SOCK_DGRAM; /* datagram socket */
    hints.ai_protocol   = IPPROTO_UDP;

    hints.ai_flags |= AI_ALL; // Query both IP6 and IP4 with AI_V4MAPPED

    // Resolve the server address and port
    char szPort[100];
    snprintf(szPort, sizeof(szPort), "%d", port);
    int result = getaddrinfo(szAddr, szPort, &hints, &mpAddrInfo);
    if (result != 0) {
      LOG_TRACE(("getaddrinfo failed with error: %d \r\n", result));
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
      exit(EXIT_FAILURE);
    }
        
    //Create a socket
    if((mSocket = socket(mpAddrInfo->ai_family, mpAddrInfo->ai_socktype, mpAddrInfo->ai_protocol )) == INVALID_SOCKET){
      auto err = WSAGetLastError();
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
      exit(-1);
    }
    printf("Socket created.\n");
  }

  // //////////////////////////////////////////////////////////////////////////
  ~SerSocketQueue(){
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
      fprintf(stderr, "send: %s\n", gai_strerror(err));
      //printf("sendto() failed with error code : %d" , WSAGetLastError());
      exit(EXIT_FAILURE);
    }

    if (ok) {
      static int timeout = 1000;
      stat = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
      ok = (stat != SOCKET_ERROR);
    }

    if (ok){

      // Use thread to read from socket to prevent blocking.
      auto readFromSocketThreadC = [](void *pThis) {
        SerSocketQueue &ssq = *(SerSocketQueue *)pThis;
        int server_length = ssq.mpAddrInfo->ai_addrlen;
        int rx = recvfrom(ssq.mSocket, (char *)ssq.mRxBuf, sizeof(ssq.mRxBuf), 0, ssq.mpAddrInfo->ai_addr, &server_length);
        if (SOCKET_ERROR != rx) {
          ssq.mRxByteQ.Write(ssq.mRxBuf, rx);
        }
        else {
          const int err = WSAGetLastError();
          fprintf(stderr, "recv: %s\n", gai_strerror(err));
        }
      };

      std::thread t(readFromSocketThreadC, this);
      t.detach();
    }
    return (ok) ? len : 0;
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t Read(uint8_t arr[], const size_t len) override {
    return mRxByteQ.Read(arr, len);
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t GetWriteReady() override {
    return 2048;
  }
  
  // //////////////////////////////////////////////////////////////////////////
  size_t GetReadReady() override {
    return mRxByteQ.GetReadReady();
  }

private:
  // /////////////////////////////////
  void readFromSocketThread()
  {
  }

private:
  struct addrinfo  *mpAddrInfo;
  SOCKET            mSocket;
  ByteQ             mRxByteQ;
  uint8_t           mRxBuf[DEFAULT_BUFLEN];
};

// ////////////////////////////////////////////////////////////////////////////
QueueBase &CreateUdpClient(const char *szAddr, const int port){
  auto p = new SerSocketQueue(szAddr, port);
  return *p;
}

// ////////////////////////////////////////////////////////////////////////////
void DeleteUdpClient(QueueBase *p){
  SerSocketQueue *ps = (SerSocketQueue *)p;
  delete ps;
}
