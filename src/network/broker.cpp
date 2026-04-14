#include "broker.h"
#include <iostream>
#include <cstring>

static std::string sock_key(socket_t s) {
#ifdef _WIN32
    return std::to_string((unsigned long long)s);
#else
    return std::to_string((long long)s);
#endif
}

static int GLOBALNI_ID_SUBSCRIBERA = 1;

Broker::Broker() {
    teme = new HashMap();
    sockToSub = new HashMap();
}

Broker::~Broker() {
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        HashMapCvor* c = teme->korpa[i];
        while (c) {
            HashMapCvor* nx = c->sledeci;
            Topic* t = (Topic*)c->vrijednost;
            delete t;
            delete c;
            c = nx;
        }
    }
    delete teme;

    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        HashMapCvor* c = sockToSub->korpa[i];
        while (c) {
            HashMapCvor* nx = c->sledeci;
            Subscriber* s = (Subscriber*)c->vrijednost;
            delete s;
            delete c;
            c = nx;
        }
    }
    delete sockToSub;
}

void Broker::run(int port) {
    if (!net_init()) {
        std::cout << "Network init faild\n";
        return;
    }

    socket_t server = create_server_socket(port);
    if (server == INVALID_SOCKET) {
        std::cout << "Cannot create server socket on port " << port << "\n";
        net_cleanup();
        return;
    }

    std::cout << "Broker pokrenut na portu: " << port << "\n";
    std::cout << "Ceka klijente... \n";

    while (true) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int len = sizeof(clientAddr);
#else
        socklen_t len = sizeof(clientAddr);
#endif
        socket_t client = accept(server, (sockaddr*)&clientAddr, &len);
        if (client == INVALID_SOCKET) continue;

        std::thread th(&Broker::client_thread, this, client);
        th.detach();
    }
}

void Broker::client_thread(socket_t clientSock) {
    char buf[2048];
    std::string pending;

    send_line(clientSock, "OK Connected\n");

    while (true) {
        int n = recv_some(clientSock, buf, (int)sizeof(buf));
        if (n <= 0) break;

        pending.append(buf, buf + n);

        size_t pos;
        while ((pos = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);

            if (!line.empty() && line.back() == '\r') line.pop_back();

            handle_line(clientSock, line);
        }
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        remove_subscriber_for_socket(clientSock);
    }
    CLOSESOCK(clientSock);
}

bool Broker::split_path(const std::string& path, std::string& outTema, std::string& outPodtema) {
    size_t p = path.find('/');
    if (p == std::string::npos) {
        outTema = path;
        outPodtema.clear();
        return true;
    }
    outTema = path.substr(0, p);
    outPodtema = path.substr(p + 1);
    return !outTema.empty() && !outPodtema.empty();
}

Topic* Broker::get_topic(const std::string& name) {
    return (Topic*)teme->get(name);
}

Topic* Broker::get_or_create_topic(const std::string& name) {
    Topic* t = (Topic*)teme->get(name);
    if (t) return t;
    t = new Topic(name);
    teme->put(name, t);
    return t;
}

Subscriber* Broker::get_subscriber_for_socket(socket_t clientSock) {
    return (Subscriber*)sockToSub->get(sock_key(clientSock));
}

Subscriber* Broker::get_or_create_subscriber_for_socket(socket_t clientSock, const std::string& nameIfNew) {
    Subscriber* s = get_subscriber_for_socket(clientSock);
    if (s) return s;
    s = new Subscriber(nameIfNew.empty() ? "anon" : nameIfNew, clientSock);
    s->id = GLOBALNI_ID_SUBSCRIBERA++;
    sockToSub->put(sock_key(clientSock), s);
    return s;
}

void Broker::remove_subscriber_for_socket(socket_t clientSock) {
    std::string key = sock_key(clientSock);
    Subscriber* s = (Subscriber*)sockToSub->get(key);
    if (s) {
        sockToSub->remove(key);
        delete s;
    }
}

bool Broker::subscribe_path(socket_t clientSock, const std::string& path) {
    std::string temaName, podName;
    if (!split_path(path, temaName, podName)) return false;

    Subscriber* sub = get_or_create_subscriber_for_socket(clientSock, "sub");

    Topic* tema = get_topic(temaName);
    if (!tema) return false;

    if (podName.empty()) {
        tema->dodajSubskrajbera(sub);
        return true;
    }

    Topic* pod = tema->pronadjiPodtemu(podName);
    if (!pod) return false;

    pod->dodajSubskrajbera(sub);
    return true;
}

bool Broker::unsubscribe_path(socket_t clientSock, const std::string& path) {
    std::string temaName, podName;
    if (!split_path(path, temaName, podName)) return false;

    Subscriber* sub = get_subscriber_for_socket(clientSock);
    if (!sub) return false;

    Topic* tema = get_topic(temaName);
    if (!tema) return false;

    if (podName.empty()) {
        tema->ukloniSubskrajbera(sub);
        return true;
    }

    Topic* pod = tema->pronadjiPodtemu(podName);
    if (!pod) return false;
    pod->ukloniSubskrajbera(sub);
    return true;
}

static void notify_list(ListaSubskrajbera* lista, const std::string& text) {
    if (!lista) return;
    ListaSubskrajberaCvor* c = lista->glava;
    while (c) {
        if (c->subskrajber) c->subskrajber->primiPoruku(text);
        c = c->sledeci;
    }
}

bool Broker::publish_path(const std::string& path, const std::string& msg) {
    std::string temaName, podName;
    if (!split_path(path, temaName, podName))
        return false;

    Topic* tema = get_or_create_topic(temaName);

    if (podName.empty()) {
        notify_list(tema->subskrajberi,
            "Nova poruka na temu " + temaName + ": " + msg);

        for (int i = 0; i < HASH_MAP_SIZE; i++) {
            HashMapCvor* c = tema->podTeme->korpa[i];
            while (c) {
                Topic* pod = (Topic*)c->vrijednost;
                notify_list(pod->subskrajberi,
                    "Nova poruka na temi " + temaName + "/" + pod->nazivTeme + ": " + msg);
                c = c->sledeci;
            }
        }
        return true;
    }

    Topic* pod = tema->pronadjiPodtemu(podName);
    if (!pod) {
        pod = new Topic(podName);
        tema->dodajPodtemu(pod);
    }

    notify_list(pod->subskrajberi,
        "Nova poruka na temi " + temaName + "/" + podName + ": " + msg);

    return true;
}

void Broker::delete_path(const std::string& path) {
    std::string temaName, podName;
    if (!split_path(path, temaName, podName)) return;

    Topic* tema = get_topic(temaName);
    if (!tema) return;

    if (podName.empty()) {
        
        notify_list(tema->subskrajberi, "DELETED " + temaName);

        for (int i = 0; i < HASH_MAP_SIZE; i++) {
            HashMapCvor* c = tema->podTeme->korpa[i];
            while (c) {
                Topic* pod = (Topic*)c->vrijednost;
                notify_list(pod->subskrajberi, "DELETED " + temaName + "/" + pod->nazivTeme);
                c = c->sledeci;
            }
        }

        teme->remove(temaName);
        
        delete tema;
        return;
    }

    Topic* pod = tema->pronadjiPodtemu(podName);
    if (!pod) return;

    notify_list(pod->subskrajberi,  "DELETED " + temaName + "/" + podName);
    notify_list(tema->subskrajberi, "DELETED " + temaName + "/" + podName);

    tema->ukloniPodtemu(podName);
}

void Broker::handle_line(socket_t clientSock, const std::string& line) {
    size_t sp = line.find(' ');
    std::string cmd = (sp == std::string::npos) ? line : line.substr(0, sp);
    std::string rest = (sp == std::string::npos) ? "" : line.substr(sp + 1);

    std::lock_guard<std::mutex> lock(mtx);

    if (cmd == "ID") {
        std::string name = rest.empty() ? "sub" : rest;
        Subscriber* s = get_or_create_subscriber_for_socket(clientSock, name);
        (void)s;
        send_line(clientSock, "OK ID\n");
        return;
    }

    if (cmd == "SUB") {
        if (subscribe_path(clientSock, rest))
            send_line(clientSock, "OK SUB\n");
        else
            send_line(clientSock, "ERR SUB (tema/podtema ne postoji)\n");
        return;
    }

    if (cmd == "UNSUB") {
        if (unsubscribe_path(clientSock, rest))
            send_line(clientSock, "OK UNSUB\n");
        else
            send_line(clientSock, "ERR UNSUB\n");
        return;
    }

    if (cmd == "PUB") {
        size_t sp2 = rest.find(' ');
        if (sp2 == std::string::npos) {
            send_line(clientSock, "GRESKA: Pogresan format poruke\n");
            return;
        }

        std::string path = rest.substr(0, sp2);
        std::string msg  = rest.substr(sp2 + 1);

        if (publish_path(path, msg)) {
            send_line(clientSock, "Poruka uspesno objavljena\n");
        } else {
            send_line(clientSock, "GRESKA: Neuspelo objavljivanje\n");
        }
        return;
    }

    if (cmd == "DEL") {
        delete_path(rest);
        send_line(clientSock, "OK DEL\n");
        return;
    }
if (cmd == "LIST") {
        std::string out = "=== TEME I PODTEME ===\n";
        bool imaTema = false;

        for (int i = 0; i < HASH_MAP_SIZE; i++) {
            HashMapCvor* c = teme->korpa[i];
            while (c) {
                imaTema = true;
                Topic* t = (Topic*)c->vrijednost;
                out += "- " + t->nazivTeme + "\n";

                for (int j = 0; j < HASH_MAP_SIZE; j++) {
                    HashMapCvor* pc = t->podTeme->korpa[j];
                    while (pc) {
                        Topic* pod = (Topic*)pc->vrijednost;
                        out += "  * " + pod->nazivTeme + "\n";
                        pc = pc->sledeci;
                    }
                }
                c = c->sledeci;
            }
        }

        if (!imaTema) {
            out += "(Nema dostupnih tema)\n";
        }
        
        out += "=== KRAJ LISTE ===\n";  
        send_line(clientSock, out.c_str());
        return;
    }

    if (cmd == "INFO") {
        send_line(clientSock, "=== PRETPLATE ===\n");  
        ispisiSvePretplate(clientSock);
        send_line(clientSock, "=== KRAJ PRETPLATA ===\n");  
        return;
    }


    if (cmd == "CREATE") {
        if (create_path(rest))
            send_line(clientSock, "Tema uspesno kreirana\n");
        else
            send_line(clientSock, "GRESKA: Tema ili podtema vec postoji ili ne postoji roditelj\n");
        return;
    }


    send_line(clientSock, "ERR UNKNOWN\n");
}

bool Broker::create_path(const std::string& path) {
    std::string temaName, podName;
    if (!split_path(path, temaName, podName))
        return false;

    if (podName.empty()) {
        if (get_topic(temaName))
            return false; 

        Topic* t = new Topic(temaName);
        teme->put(temaName, t);
        return true;
    }

    Topic* tema = get_topic(temaName);
    if (!tema)
        return false;

    if (tema->pronadjiPodtemu(podName))
        return false;

    Topic* pod = new Topic(podName);
    tema->dodajPodtemu(pod);
    return true;
}


void Broker::ispisiSvePretplate(socket_t adminSock) {
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        HashMapCvor* c = teme->korpa[i];
        while (c) {
            Topic* t = (Topic*)c->vrijednost;

            t->ispisiPretplate(adminSock, t->nazivTeme);

            for (int j = 0; j < HASH_MAP_SIZE; j++) {
                HashMapCvor* pc = t->podTeme->korpa[j];
                while (pc) {
                    Topic* pod = (Topic*)pc->vrijednost;
                    pod->ispisiPretplate(
                        adminSock,
                        t->nazivTeme + "/" + pod->nazivTeme
                    );
                    pc = pc->sledeci;
                }
            }

            c = c->sledeci;
        }
    }
}
