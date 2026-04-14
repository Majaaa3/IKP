#include "hash_map.h"

HashMap::HashMap(){
    for(int i=0; i<HASH_MAP_SIZE; i++){
        korpa[i]=nullptr;
    }
}

HashMap::~HashMap(){  
    for(int i=0; i<HASH_MAP_SIZE; i++){
        HashMapCvor* tekuci = korpa[i];
        while(tekuci != nullptr){
            HashMapCvor* sledeci = tekuci->sledeci;
            delete tekuci;
            tekuci = sledeci;
        }
        korpa[i] = nullptr;  
    }
}

int HashMap::hash(const std::string& kljuc){
    int suma=0;
    for(char c : kljuc){
        suma+=c;
    }
    return suma % HASH_MAP_SIZE;
}

void HashMap::put(const std::string& kljuc, void* value){
    int index=hash(kljuc);
    HashMapCvor* tekuci=korpa[index];

    while(tekuci!=nullptr){
        if(tekuci->kljuc==kljuc){
            tekuci->vrijednost=value;
            return;
        }
        tekuci=tekuci->sledeci;
    }

    HashMapCvor* noviCvor=new HashMapCvor;
    noviCvor->kljuc=kljuc;
    noviCvor->vrijednost=value;
    noviCvor->sledeci=korpa[index];
    korpa[index]=noviCvor;
}

bool HashMap::contains(const std::string& kljuc){
    int index=hash(kljuc);
    HashMapCvor* tekuci=korpa[index];

    while(tekuci!=nullptr){
        if(tekuci->kljuc==kljuc){
            return true;
        }
        tekuci=tekuci->sledeci;
    }
    return false;
}

void HashMap::remove(const std::string& kljuc){
    int index=hash(kljuc);
    HashMapCvor* tekuci=korpa[index];
    HashMapCvor* prethodni=nullptr;

    while(tekuci!=nullptr){
        if(tekuci->kljuc==kljuc){
            if(prethodni==nullptr){
                korpa[index]=tekuci->sledeci;
                delete tekuci;
                return;
            }else{
                prethodni->sledeci=tekuci->sledeci;
            }
            delete tekuci;
            return;
        }
        prethodni=tekuci;
        tekuci=tekuci->sledeci;
    }
}

void* HashMap::get(const std::string& kljuc){
    int index=hash(kljuc);
    HashMapCvor* tekuci=korpa[index];

    while(tekuci!=nullptr){
        if(tekuci->kljuc==kljuc){
            return tekuci->vrijednost;
        }
        tekuci=tekuci->sledeci;
    }
    return nullptr;
}
