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

// /////////////////////////////////
class SerSocketQueue {
private:
  SOCKET mListenSocket; ///< Wait for connections on this socket.
  SOCKET mNextCommsSocket; /// Receive data on this socket.
  ByteQ   mRxByteQ;
  volatile bool mRunning;
  volatile int mSocketCount;
  int mPort;
  std::thread *mpAcceptThread;
  const size_t recvbuflen;
  uint8_t recvbuf[DEFAULT_BUFLEN];


public:
  // /////////////////////////////////
  SerSocketQueue
  (const int port)
    : mListenSocket(INVALID_SOCKET)
    , mNextCommsSocket(INVALID_SOCKET)
    , mRxByteQ()
    , mRunning(true)
    , mSocketCount(0)
    , mPort(port)
    , mpAcceptThread(nullptr)
    , recvbuflen(sizeof(recvbuf))
    , recvbuf{ 0 }
  {
    SerSocket::inst().Init();
    setupPort(true, mPort);

    // Create a thread for listening for incoming connections.
    mpAcceptThread = new std::thread(nextRxConnectionC, this);
    mpAcceptThread->detach();

  }

  // /////////////////////////////////
  ~SerSocketQueue() {
    SerSocket::inst().sockClose(mListenSocket);
    mRunning = false;
    mSocketCount += 100;
    // No longer need server socket
    mpAcceptThread->join();
    delete mpAcceptThread;
  }

private:

  void setupPort(const bool isRxPort, const int port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    struct addrinfo *rxResult = NULL;
    char szPort[100];
    snprintf(szPort, sizeof(szPort), "%d", port);
    int result = getaddrinfo(NULL, szPort, &hints, &rxResult);
    if (result != 0) {
      LOG_TRACE(("getaddrinfo failed with error: %d \r\n", result));
      return;
    }

    if (isRxPort) {
      // Create a SOCKET for connecting to server
      mListenSocket = socket(rxResult->ai_family, rxResult->ai_socktype, rxResult->ai_protocol);
      if (mListenSocket == INVALID_SOCKET) {
        LOG_TRACE(("socket failed with error \r\n"));
        freeaddrinfo(rxResult);
        return;
      }

      // Setup the TCP listening socket
      result = bind(mListenSocket, rxResult->ai_addr, (int)rxResult->ai_addrlen);
      if (result == SOCKET_ERROR) {
        LOG_TRACE(("listen failed with error \r\n"));
        freeaddrinfo(rxResult);
        SerSocket::inst().sockClose(mListenSocket);
        return;
      }

      freeaddrinfo(rxResult);

      result = listen(mListenSocket, SOMAXCONN);
      if (result == SOCKET_ERROR) {
        LOG_TRACE(("listen failed with error \r\n"));
        SerSocket::inst().sockClose(mListenSocket);
        return;
      }
    }
  }

  // /////////////////////////////////
  static void readFromSocketThreadC(void *pThis) {
    ((SerSocketQueue *)pThis)->readFromSocketThread();
  }

  // /////////////////////////////////
  void readFromSocketThread()
  {
    const int socketCount = mSocketCount;
    SOCKET clientSocket = mNextCommsSocket;
    // Receive until the peer shuts down the connection
    int bytesReceived = 1;
    while ((mNextCommsSocket != INVALID_SOCKET) && (bytesReceived > 0) && (socketCount == mSocketCount)) {
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

    if (clientSocket == mNextCommsSocket) {
      // shutdown the connection since we're done
      bytesReceived = SerSocket::inst().sockClose(clientSocket);
      if (bytesReceived == SOCKET_ERROR) {
        LOG_TRACE(("shutdown failed with error\r\n"));
      }
      LOG_TRACE(("readFromSocketThread thread exiting.\r\n"));
    }
  }

  // /////////////////////////////////
  static void nextRxConnectionC(void *pThis) {
    ((SerSocketQueue *)pThis)->nextRxConnection();
  }

  // /////////////////////////////////
  // This thread constantly accepts new connections
  void nextRxConnection() {
    while (mRunning) {
      // Accept a client socket
      mNextCommsSocket = accept(mListenSocket, NULL, NULL);
      if (mNextCommsSocket == INVALID_SOCKET) {
        auto what = GetLastError();
        LOG_TRACE(("accept failed with error %u\r\n", what));
      }
      else {
        {
          CSTaskLocker cs;
          mSocketCount++;
        }
        std::thread t(readFromSocketThreadC, this); // pass by reference
        t.detach();
      }
      OSALSleep(1000);
    }
  }

  typedef struct SendBufTag {
    struct {
      uint32_t numBytes;
      SerSocketQueue *pThis;
    } hdr;
    uint8_t payload[4];
  } SendBufT;

  // /////////////////////////////////
  void sendThread(SendBufT *p) {

    // Handle transmission on the send socket.
    if (mNextCommsSocket != INVALID_SOCKET) {
      SOCKET clientSocket = mNextCommsSocket;
      int result = send(clientSocket, (char *)p->payload, p->hdr.numBytes, 0);
      if (result == SOCKET_ERROR) {
        mNextCommsSocket = INVALID_SOCKET;
        LOG_WARNING(("Could not send to socket.\r\n"));
        //SerSocket::inst().sockClose(clientSocket);
      }
    }
    free(p);
  }

  static void sendThreadC(void *p) {
    ((SendBufT *)p)->hdr.pThis->sendThread((SendBufT *)p);
  }

  // //////////////////////////////////////////////////////////////////////////
  // Write function, returns amount written.  Does not trigger a callback as
  // it is typically implemented using a FIFO.
  int Write(const uint8_t *const pBytes, const int numBytes) {
    if (mNextCommsSocket != INVALID_SOCKET) {
      SendBufT *pThreadData = (SendBufT *)malloc(sizeof(SendBufT::hdr) + numBytes);
      pThreadData->hdr.pThis = this;
      pThreadData->hdr.numBytes = numBytes;
      memcpy(pThreadData->payload, pBytes, numBytes);
      std::thread t(sendThreadC, pThreadData);
      t.detach();
    }
    return numBytes;
  }

  // //////////////////////////////////////////////////////////////////////////
  // Get Write Ready.  Returns amount that can be written.
  int GetWriteReady() {
    return DEFAULT_BUFLEN;
  }

  // //////////////////////////////////////////////////////////////////////////
  // Read function - returns amount available.  Typically uses an internal FIFO.
  int Read(uint8_t *const pBytes, const int numBytes) {
    CSTaskLocker cs;
    return mRxByteQ.Read(pBytes, numBytes);
  }

  // Get the number of bytes that can be read.
  int GetReadReady() {
    CSTaskLocker cs;
    return mRxByteQ.GetReadReady();
  }

};

