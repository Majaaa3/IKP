#include "topic.h"

#include "../utils/hash_map.h"       
#include "../utils/subscriber_list.h"
#include "model/subscriber.h"

Topic::Topic(const std::string& nazivTeme)
    : nazivTeme(nazivTeme),
      podTeme(new HashMap()),
      subskrajberi(new ListaSubskrajbera())
{
}

Topic::~Topic(){ 
    for(int i = 0; i < HASH_MAP_SIZE; i++){
        HashMapCvor* cvor = podTeme->korpa[i];
        while(cvor != nullptr){
            Topic* podtema = (Topic*)cvor->vrijednost;
            HashMapCvor* sledeci = cvor->sledeci;
            
            delete podtema; 
            
            cvor = sledeci;
        }
        podTeme->korpa[i] = nullptr;
    }
    delete podTeme;  
    
    ListaSubskrajberaCvor* tekuci = subskrajberi->glava;
    while(tekuci != nullptr){
        ListaSubskrajberaCvor* sledeci = tekuci->sledeci;
        delete tekuci;  
        tekuci = sledeci;
    }
    subskrajberi->glava = nullptr;  
    delete subskrajberi;
}


bool Topic::dodajPodtemu(Topic* podtema){
    if(podtema==nullptr){
        return false;
    }

    if(podTeme->contains(podtema->nazivTeme)){
        return false;
    }

    podTeme->put(podtema->nazivTeme, podtema);
    return true;
}

bool Topic::ukloniPodtemu(const std::string& nazivPodteme){
    if(!podTeme->contains(nazivPodteme)){
        return false;
    }

    Topic* podtema = (Topic*) podTeme->get(nazivPodteme);

    podTeme->remove(nazivPodteme);

    delete podtema;

    return true;
}

Topic* Topic::pronadjiPodtemu(const std::string& nazivPodteme){
    if(!podTeme->contains(nazivPodteme)){
        return nullptr;
    }

    Topic* podtema=(Topic*)podTeme->get(nazivPodteme);

    return podtema;
}

void Topic::dodajSubskrajbera(Subscriber* subskrajber){
    if(subskrajber==nullptr){
        return;
    }

    if(!subskrajberi->contains(subskrajber)){
        subskrajberi->add(subskrajber);
    }
}

void Topic::ukloniSubskrajbera(Subscriber* subskrajber){
    if(subskrajber==nullptr){
        return;
    }

    subskrajberi->remove(subskrajber);
}

void Topic::ispisiPretplate(socket_t adminSock, const std::string& punaPutanja) {
    ListaSubskrajberaCvor* tekuci = subskrajberi->glava;
    
    bool imaPretplata = false;
    while (tekuci != nullptr) {
        Subscriber* s = tekuci->subskrajber;
        if (s) {
            imaPretplata = true;
            std::string poruka =
                "  [" + punaPutanja + "] " +
                "ID: " + std::to_string(s->id) +
                " | " + s->ime + "\n";
            send_line(adminSock, poruka.c_str());
        }
        tekuci = tekuci->sledeci;
    }
    
}