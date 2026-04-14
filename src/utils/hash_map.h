#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <string>

#define HASH_MAP_SIZE 15

struct HashMapCvor{
    std::string kljuc;
    void* vrijednost;
    HashMapCvor* sledeci;
};

struct HashMap{
    HashMapCvor* korpa[HASH_MAP_SIZE];

    HashMap();
    ~HashMap();

    int hash(const std::string& key);
    void put(const std::string& kljuc, void* value);
    bool contains(const std::string &key);
    void remove(const std::string& kljuc);
    void* get(const std::string& kljuc);
};

#endif