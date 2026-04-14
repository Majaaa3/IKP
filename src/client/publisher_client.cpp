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

            if (line.find("OK ") == 0 || line == "OK Connected") {
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
        std::cout << "Network init failed\n";
        return 1;
    }

    socket_t sock = connect_to_server(ip, port);
    if (sock == INVALID_SOCKET) {
        std::cout << "Cannot connect to broker\n";
        return 1;
    }

    send_line(sock, "ID publisher\n");

    std::thread recvThread(recv_loop, sock);
    recvThread.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (true) {
        std::cout << "\n--- PUBLISHER MENU ---\n";
        std::cout << "1. Kreiraj temu\n";
        std::cout << "2. Kreiraj podtemu\n";
        std::cout << "3. Objavi poruku\n";
        std::cout << "0. Izlaz\n";
        std::cout << "Izbor: ";

        int izbor;
        std::cin >> izbor;
        std::cin.ignore();

        if (izbor == 1) {
            std::string tema;
            std::cout << "Unesite naziv teme: ";
            std::getline(std::cin, tema);

            std::string cmd = "CREATE " + tema + "\n";
            send_line(sock, cmd.c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else if (izbor == 2) {
            std::string tema, podtema;
            std::cout << "Unesite naziv teme: ";
            std::getline(std::cin, tema);
            std::cout << "Unesite naziv podteme: ";
            std::getline(std::cin, podtema);

            std::string cmd = "CREATE " + tema + "/" + podtema + "\n";
            send_line(sock, cmd.c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else if (izbor == 3) {
            std::string path, poruka;
            std::cout << "Unesite temu ili temu/podtemu: ";
            std::getline(std::cin, path);
            std::cout << "Unesite poruku: ";
            std::getline(std::cin, poruka);

            std::string cmd = "PUB " + path + " " + poruka + "\n";
            send_line(sock, cmd.c_str());
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else if (izbor == 0) {
            break;
        }
        else {
            std::cout << "Nepostojeca opcija.\n";
        }
    }

    CLOSESOCK(sock);
    net_cleanup();
    return 0;
}