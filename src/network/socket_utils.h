#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#ifdef _WIN32
  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <errno.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define CLOSESOCK close
#endif

bool net_init();
void net_cleanup();

socket_t create_server_socket(int port, int backlog = 16);
socket_t connect_to_server(const char* ip, int port);

bool send_line(socket_t s, const char* line);
int recv_some(socket_t s, char* buf, int bufSize);

#endif
