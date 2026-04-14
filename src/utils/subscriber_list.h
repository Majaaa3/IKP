
#ifndef SUBSCRIBER_LIST_H
#define SUBSCRIBER_LIST_H

struct Subscriber;

struct ListaSubskrajberaCvor{
    Subscriber* subskrajber;
    ListaSubskrajberaCvor* sledeci;
};

struct ListaSubskrajbera{
    ListaSubskrajberaCvor* glava;

    ListaSubskrajbera();
    ~ListaSubskrajbera();

    bool contains(Subscriber* subskrajber);
    void add(Subscriber* subskrajber);
    void remove(Subscriber* subskrajber);
};

#endif
