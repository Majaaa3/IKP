#include "socket_utils.h"
#include <cstring>

bool net_init() {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2,2), &wsa) == 0;
#else
    return true;
#endif
}

void net_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

socket_t create_server_socket(int port, int backlog) {
    socket_t s = ::socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;

    int opt = 1;
#ifdef _WIN32
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (::bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        CLOSESOCK(s);
        return INVALID_SOCKET;
    }
    if (::listen(s, backlog) == SOCKET_ERROR) {
        CLOSESOCK(s);
        return INVALID_SOCKET;
    }
    return s;
}

socket_t connect_to_server(const char* ip, int port) {
    socket_t s = ::socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr(ip);
#else
    inet_pton(AF_INET, ip, &addr.sin_addr);
#endif

    if (::connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        CLOSESOCK(s);
        return INVALID_SOCKET;
    }
    return s;
}

bool send_line(socket_t s, const char* line) {
    const int len = (int)std::strlen(line);
    int sent = ::send(s, line, len, 0);
    if (sent != len) return false;
    if (len == 0 || line[len-1] != '\n') {
        const char nl = '\n';
        return ::send(s, &nl, 1, 0) == 1;
    }
    return true;
}

int recv_some(socket_t s, char* buf, int bufSize) {
    return (int)::recv(s, buf, bufSize, 0);
}
