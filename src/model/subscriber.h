#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <string>
#include "network/socket_utils.h"

struct Subscriber {
    int id;
    std::string ime;
    socket_t sock;

    Subscriber(const std::string& ime, socket_t sock);

    void primiPoruku(const std::string& poruka);
};

#endif
