#include "subscriber_list.h"

ListaSubskrajbera::ListaSubskrajbera(){
    glava=nullptr;
}

ListaSubskrajbera::~ListaSubskrajbera(){  
    ListaSubskrajberaCvor* tekuci = glava;
    while(tekuci != nullptr){
        ListaSubskrajberaCvor* sledeci = tekuci->sledeci;
        delete tekuci;  
        tekuci = sledeci;
    }
}

bool ListaSubskrajbera::contains(Subscriber* subskrajber){
    ListaSubskrajberaCvor* tekuci= glava;

    while(tekuci!=nullptr){
        if(tekuci->subskrajber==subskrajber){
            return true;
        }
        tekuci=tekuci->sledeci;
    }
    return false;
}

void ListaSubskrajbera::add(Subscriber* subskrajber){
    if(contains(subskrajber)){
        return;
    }

    ListaSubskrajberaCvor* noviCvor=new ListaSubskrajberaCvor();
    noviCvor->subskrajber=subskrajber;
    noviCvor->sledeci=glava;
    glava=noviCvor;
}

void ListaSubskrajbera::remove(Subscriber* subskrajber){
    ListaSubskrajberaCvor* tekuci=glava;
    ListaSubskrajberaCvor* prethodni=nullptr;

    while(tekuci!=nullptr){
        if(tekuci->subskrajber==subskrajber){
            if(prethodni==nullptr){
                glava=tekuci->sledeci;
                delete tekuci;  
                return;
            }else{
                prethodni->sledeci=tekuci->sledeci;
                delete tekuci;
                return;
            }
        }
        prethodni=tekuci;
        tekuci=tekuci->sledeci;
    }
}