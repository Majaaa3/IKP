#include "subscriber.h"
#include <string>

Subscriber::Subscriber(const std::string& ime, socket_t sock)
    : id(-1), ime(ime), sock(sock) {}

void Subscriber::primiPoruku(const std::string& poruka) {
    std::string line = "Poruka " + poruka + "\n";
    send_line(sock, line.c_str());
}
