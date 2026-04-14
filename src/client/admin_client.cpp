#include "network/socket_utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static void recv_loop(socket_t sock) {
    char buf[4096];
    std::string pending;

    while (true) {
        int n = recv_some(sock, buf, sizeof(buf));
        if (n <= 0) break;

        pending.append(buf, buf + n);

        size_t pos;
        while ((pos = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);

            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.find("OK ") == 0) {
                continue;  
            }

            std::cout << line << std::endl;
        }
    }
}

int main() {
    const char* ip = "127.0.0.1";
    int port = 5555;

    if (!net_init()) {
        std::cout << "Neuspjela inicijalizacija mreze.\n";
        return 1;
    }

    socket_t sock = connect_to_server(ip, port);
    if (sock == INVALID_SOCKET) {
        std::cout << "Ne mogu da se povezem na broker.\n";
        return 1;
    }

    send_line(sock, "ID admin\n");

    std::thread recvThread(recv_loop, sock);
    recvThread.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (true) {
        std::cout << "\n--- ADMIN MENU ---\n";
        std::cout << "1. Obrisi temu\n";
        std::cout << "2. Obrisi podtemu\n";
        std::cout << "3. Lista tema\n";
        std::cout << "4. Prikazi pretplate\n";
        std::cout << "0. Izlaz\n";
        std::cout << "Izbor: ";

        int izbor;
        std::cin >> izbor;
        std::cin.ignore();

        if (izbor == 1) {
            std::string tema;
            std::cout << "Unesite naziv teme: ";
            std::getline(std::cin, tema);
            send_line(sock, ("DEL " + tema + "\n").c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else if (izbor == 2) {
            std::string tema, podtema;
            std::cout << "Unesite naziv teme: ";
            std::getline(std::cin, tema);
            std::cout << "Unesite naziv podteme: ";
            std::getline(std::cin, podtema);
            send_line(sock, ("DEL " + tema + "/" + podtema + "\n").c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else if (izbor == 3) {
            send_line(sock, "LIST\n");
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (izbor == 4) {
            send_line(sock, "INFO\n");
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (izbor == 0) {
            break;
        }
    }

    CLOSESOCK(sock);
    net_cleanup();
    return 0;
}