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
  //const int PORT = 2002;//8888  //The port on which to listen for incoming data
  
  SerSocketQueue(const char *szAddr, const int port)
  : QueueBase()
  , mSocket(INVALID_SOCKET)
  , mRxByteQ()
  , mPort(port)
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
    int result = getaddrinfo(szAddr, szPort, &hints, &rxResult);
    if (result != 0) {
      LOG_TRACE(("getaddrinfo failed with error: %d \r\n", result));
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
      exit(EXIT_FAILURE);
    }
        
    //Initialise winsock
    printf("\nInitialising Winsock...");
    SerSocket::inst().Init();
    printf("Initialised.\n");
    
    //Create a socket
    if((mSocket = socket(rxResult->ai_family, rxResult->ai_socktype, rxResult->ai_protocol )) == INVALID_SOCKET){
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(mSocket));
      exit(-1);
    }
    printf("Socket created.\n");
  }

  // //////////////////////////////////////////////////////////////////////////
  ~SerSocketQueue(){
    freeaddrinfo(rxResult);    
  }

  // //////////////////////////////////////////////////////////////////////////
  size_t Write(const uint8_t arr[], const size_t len) override {

    if (sendto(mSocket, (const char *)arr, len, 0, rxResult->ai_addr, rxResult->ai_addrlen) == SOCKET_ERROR) {
      auto err = WSAGetLastError();
      fprintf(stderr, "send: %s\n", gai_strerror(err));
      //printf("sendto() failed with error code : %d" , WSAGetLastError());
      exit(EXIT_FAILURE);
    }
    else {
      static int timeout = 100;
      if (0 == setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))){
        int server_length = rxResult->ai_addrlen;
        auto rx = recvfrom(mSocket, (char *)mRxBuf, sizeof(mRxBuf), 0, rxResult->ai_addr, &server_length);
        if (SOCKET_ERROR != rx) {
          mRxByteQ.Write(mRxBuf, rx);
        }
        else {
          auto err = WSAGetLastError();
          fprintf(stderr, "recv: %s\n", gai_strerror(err));
          exit(EXIT_FAILURE);
        }
      }
    }
    return 0;
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
  struct addrinfo *rxResult;
  SOCKET mSocket; ///< Wait for connections on this socket.
  ByteQ   mRxByteQ;
  int mPort;
  uint8_t mRxBuf[DEFAULT_BUFLEN];
};

// ////////////////////////////////////////////////////////////////////////////
QueueBase &GetQueueToIp(const char *szAddr, const int port){
  auto p = new SerSocketQueue(szAddr, port);
  return *p;
}

// ////////////////////////////////////////////////////////////////////////////
void ReleaseQueueToIp(QueueBase *p){
  SerSocketQueue *ps = (SerSocketQueue *)p;
  delete ps;
}
