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

static const int DEFAULT_BUFLEN = 32768;
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
  , mRunning(true)
  , mPort(port)
  , recvbuflen(sizeof(recvbuf))
  , recvbuf{ 0 }
  {
    
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    //hints.ai_flags = AI_PASSIVE;
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=0;
    hints.ai_protocol = IPPROTO_UDP;
    
    // Resolve the server address and port
    struct addrinfo *rxResult = NULL;
    char szPort[100];
    snprintf(szPort, sizeof(szPort), "%d", port);
    int result = getaddrinfo(szAddr, szPort, &hints, &rxResult);
    if (result != 0) {
      LOG_TRACE(("getaddrinfo failed with error: %d \r\n", result));
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
      return;
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
#if 1
#if 1
    //Prepare the sockaddr_in structure
    static struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( mPort );
    
    //Bind
    if( bind(mSocket ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
#else
    if ( bind(mSocket, rxResult->ai_addr, rxResult->ai_addrlen) == SOCKET_ERROR)
#endif
    {
      //printf("Bind failed with error code : %d" , WSAGetLastError());
      fprintf(stderr, "bind: %s\n", gai_strerror(mSocket));
      exit(EXIT_FAILURE);
    }
    puts("Bind done");
#endif

    std::thread t(readFromSocketThreadC, this); // pass by reference
    t.detach();
    
    if( connect(mSocket, rxResult->ai_addr, rxResult->ai_addrlen) == SOCKET_ERROR){
      fprintf(stderr, "connect: %s\n", gai_strerror(mSocket));
      exit(EXIT_FAILURE);
    }
    
  }

  ~SerSocketQueue(){
    mRunning = false;
  }

  size_t Write(const uint8_t arr[], const size_t len) override {
    CSTaskLocker cs;
    if (send(mSocket, arr, len, 0) == SOCKET_ERROR)
    {
      fprintf(stderr, "send: %s\n", gai_strerror(mSocket));
      //printf("sendto() failed with error code : %d" , WSAGetLastError());
      exit(EXIT_FAILURE);
    }
    return 0;
  }
  size_t Read(uint8_t arr[], const size_t len) override {
    return mRxByteQ.Read(arr, len);
  }
  size_t GetWriteReady() override {
    return 2048;
  }
  size_t GetReadReady() override {
    return mRxByteQ.GetReadReady();
  }

private:
  // /////////////////////////////////
  static void readFromSocketThreadC(void *pThis) {
    ((SerSocketQueue *)pThis)->readFromSocketThread();
  }

  // /////////////////////////////////
  void readFromSocketThread()
  {
    SOCKET clientSocket = mSocket;
    // Receive until the peer shuts down the connection
    int bytesReceived = 1;
    while ((mRunning) && (clientSocket != INVALID_SOCKET) && (bytesReceived > 0)) {
      bytesReceived = recv(clientSocket, (char *)recvbuf, recvbuflen - 1, 0);
      if (bytesReceived > 0) {
        CSTaskLocker cs;
        auto amtWritten = mRxByteQ.Write(recvbuf, bytesReceived);
        LOG_ASSERT_WARN(amtWritten == bytesReceived);
      }
      else if (bytesReceived == 0) {
        // Do nothing
      }
      else {
        LOG_TRACE(("recv failed with error\r\n"));
      }
    }
    
  }

private:
  SOCKET mSocket; ///< Wait for connections on this socket.
  ByteQ   mRxByteQ;
  volatile bool mRunning;
  int mPort;
  const size_t recvbuflen;
  uint8_t recvbuf[DEFAULT_BUFLEN];
};



QueueBase &GetQueueToIp(const char *szAddr, const int port){
  auto p = new SerSocketQueue(szAddr, port);
  return *p;
}

void ReleaseQueueToIp(QueueBase *p){
  SerSocketQueue *ps = (SerSocketQueue *)p;
  delete ps;
}
