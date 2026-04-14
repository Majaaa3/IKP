#ifndef TOPIC_H
#define TOPIC_H

#include <string>
#include "network/socket_utils.h"

struct ListaSubskrajbera;
struct HashMap;
struct Subscriber;

struct Topic {
    std::string nazivTeme;
    HashMap* podTeme;
    struct ListaSubskrajbera* subskrajberi;

    Topic(const std::string& nazivTeme);
    ~Topic();

    bool dodajPodtemu(Topic* podtema);
    bool ukloniPodtemu(const std::string& nazivPodteme);
    Topic* pronadjiPodtemu(const std::string& nazivPodteme);

    void dodajSubskrajbera(Subscriber* subskrajber);
    void ukloniSubskrajbera(Subscriber* subskrajber);

    void ispisiPretplate(socket_t adminSock, const std::string& punaPutanja);
};

#endif