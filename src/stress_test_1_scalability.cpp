
#include "network/socket_utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int g_messagesSent = 0;
int g_messagesReceived = 0;

void subscriber_function(int id, int expectedMessages) {
    socket_t sock = connect_to_server("127.0.0.1", 5555);
    if (sock == INVALID_SOCKET) {
        std::cout << "Sub" << id << ": Failed to connect\n";
        return;
    }
    
    std::string idMsg = "ID sub" + std::to_string(id) + "\n";
    send_line(sock, idMsg.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    send_line(sock, "SUB TestTopic\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    char buf[4096];
    std::string pending;
    int received = 0;
    
    while (received < expectedMessages) {
        int n = recv_some(sock, buf, sizeof(buf));
        if (n <= 0) break;
        
        pending.append(buf, n);
        
        size_t pos;
        while ((pos = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);
            
            if (line.find("Nova poruka") != std::string::npos) {
                received++;
                g_messagesReceived++;
            }
        }
    }
    
    std::cout << "Sub" << id << ": Primio " << received << " poruka\n";
    
    CLOSESOCK(sock);
}

void publisher_function(int id, int numMessages) {
    socket_t sock = connect_to_server("127.0.0.1", 5555);
    if (sock == INVALID_SOCKET) {
        std::cout << "Pub" << id << ": Failed to connect\n";
        return;
    }
    
    std::string idMsg = "ID pub" + std::to_string(id) + "\n";
    send_line(sock, idMsg.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    if (id == 0) {
        send_line(sock, "CREATE TestTopic\n");
        char buf[256];
        recv_some(sock, buf, sizeof(buf));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    char buf[1024];
    for (int i = 0; i < numMessages; i++) {
        std::string msg = "PUB TestTopic Message_" 
                        + std::to_string(id) + "_" + std::to_string(i) + "\n";
        send_line(sock, msg.c_str());
        g_messagesSent++;
        
        recv_some(sock, buf, sizeof(buf));
    }
    
    std::cout << "Pub" << id << ": Poslao " << numMessages << " poruka\n";
    
    CLOSESOCK(sock);
}

void memory_leak_test() {
    std::cout << "\n===========================================\n";
    std::cout << "MEMORY LEAK TEST\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "Ovaj test kreira 1000 konekcija i proverava\n";
    std::cout << "da li broker pravilno oslobadja memoriju.\n\n";
    
    std::cout << "KORACI:\n";
    std::cout << "Pritisni ENTER za start...\n";
    std::cin.get();
    
    const int CYCLES = 100;
    const int CLIENTS_PER_CYCLE = 10;
    
    for (int cycle = 0; cycle < CYCLES; cycle++) {
        std::thread* threads = new std::thread[CLIENTS_PER_CYCLE];
        
        for (int i = 0; i < CLIENTS_PER_CYCLE; i++) {
            threads[i] = std::thread([cycle, i]() {
                socket_t sock = connect_to_server("127.0.0.1", 5555);
                if (sock != INVALID_SOCKET) {
                    std::string id = "ID leak_" + std::to_string(cycle) 
                                   + "_" + std::to_string(i) + "\n";
                    send_line(sock, id.c_str());
                    
                    send_line(sock, "CREATE TempTopic\n");
                    char buf[256];
                    recv_some(sock, buf, sizeof(buf));
                    
                    send_line(sock, "SUB TempTopic\n");
                    recv_some(sock, buf, sizeof(buf));
                    
                    CLOSESOCK(sock);
                }
            });
        }
        
        for (int i = 0; i < CLIENTS_PER_CYCLE; i++) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
        
        delete[] threads;
        
        if ((cycle + 1) % 10 == 0) {
            std::cout << "Ciklus " << (cycle + 1) << "/" << CYCLES 
                      << " - " << ((cycle + 1) * CLIENTS_PER_CYCLE) 
                      << " konekcija\n";
        }
    }
    
    std::cout << "\nTest zavrsen!\n";
    std::cout << "Ukupno konekcija: " << (CYCLES * CLIENTS_PER_CYCLE) << "\n\n";
}


void throughput_test() {
    std::cout << "\n===========================================\n";
    std::cout << "THROUGHPUT TEST\n"; //koliko poruka sistem moze da obradi po sekundi
    std::cout << "===========================================\n\n";
    
    const int NUM_SUBSCRIBERS = 50;
    const int NUM_PUBLISHERS = 10;
    const int MESSAGES_PER_PUBLISHER = 100;
    const int TOTAL_MESSAGES = NUM_PUBLISHERS * MESSAGES_PER_PUBLISHER;
    
    g_messagesSent = 0;
    g_messagesReceived = 0;
    
    std::cout << "Konfiguracija:\n";
    std::cout << "  Subscriberi: " << NUM_SUBSCRIBERS << "\n";
    std::cout << "  Publisheri: " << NUM_PUBLISHERS << "\n";
    std::cout << "  Poruke po publisheru: " << MESSAGES_PER_PUBLISHER << "\n";
    std::cout << "  Ukupno poruka: " << TOTAL_MESSAGES << "\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread* subThreads = new std::thread[NUM_SUBSCRIBERS];
    std::thread* pubThreads = new std::thread[NUM_PUBLISHERS];
    
    std::cout << "Pokrecem subscribere...\n";
    for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
        subThreads[i] = std::thread(subscriber_function, i, TOTAL_MESSAGES);
    }
    
    std::cout << "Cekam da se subscriberi konektuju...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "Pokrecem publishere...\n";
    for (int i = 0; i < NUM_PUBLISHERS; i++) {
        pubThreads[i] = std::thread(publisher_function, i, MESSAGES_PER_PUBLISHER);
    }
    
    for (int i = 0; i < NUM_PUBLISHERS; i++) {
        if (pubThreads[i].joinable()) {
            pubThreads[i].join();
        }
    }
    
    std::cout << "Svi publisheri zavrsili!\n";
    std::cout << "Cekam subscribere...\n";
    
    for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
        if (subThreads[i].joinable()) {
            subThreads[i].join();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    delete[] subThreads;
    delete[] pubThreads;
    
    int expectedTotal = TOTAL_MESSAGES * NUM_SUBSCRIBERS;
    double successRate = (g_messagesReceived * 100.0) / expectedTotal;
    double throughput = (g_messagesSent * 1000.0) / duration.count();
    
    std::cout << "\n=== REZULTATI ===\n";
    std::cout << "Poslato: " << g_messagesSent << "\n";
    std::cout << "Primljeno: " << g_messagesReceived 
              << " (ocekivano: " << expectedTotal << ")\n";
    std::cout << "Trajanje: " << duration.count() << " ms\n";
    std::cout << "Throughput: " << throughput << " poruka/sekunda\n";
    std::cout << "Uspjesnost: " << successRate << "%\n\n";
    
    if (successRate >= 99.0) {
        std::cout << "TEST JE PROSAO! Uspjesnost > 99%\n";
    } else {
        std::cout << "TEST DJELIMICNO PROSAO. Uspjesnost < 99%\n";
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "    STRESS TESTOVI\n";
    std::cout << "========================================\n\n";
    
    if (!net_init()) {
        std::cout << "Network init failed\n";
        return 1;
    }
    
    std::cout << "Izaberi test:\n";
    std::cout << "  1 - Throughput test (performanse)\n";
    std::cout << "  2 - Memory leak test\n";
    std::cout << "  3 - Oba testa\n";
    std::cout << "Izbor: ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    
    if (choice == 1 || choice == 3) {
        throughput_test();
    }
    
    if (choice == 2 || choice == 3) {
        memory_leak_test();
    }
    
    net_cleanup();
    
    std::cout << "\nPritisni ENTER za izlaz...";
    std::cin.get();
    
    return 0;
}
