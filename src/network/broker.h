#ifndef BROKER_H
#define BROKER_H

#include <string>
#include <mutex>
#include <thread>

#include "socket_utils.h"
#include "utils/hash_map.h"
#include "utils/subscriber_list.h"
#include "model/topic.h"
#include "model/subscriber.h"

struct Broker {
    HashMap* teme;            
    HashMap* sockToSub;       

    std::mutex mtx;

    Broker();
    ~Broker();

    void run(int port);

private:
    void client_thread(socket_t clientSock);

    void handle_line(socket_t clientSock, const std::string& line);

    Topic* get_or_create_topic(const std::string& name);
    Topic* get_topic(const std::string& name);

    bool subscribe_path(socket_t clientSock, const std::string& path);
    bool unsubscribe_path(socket_t clientSock, const std::string& path);
    bool publish_path(const std::string& path, const std::string& msg);
    void delete_path(const std::string& path);

    bool split_path(const std::string& path, std::string& outTema, std::string& outPodtema);
    Subscriber* get_or_create_subscriber_for_socket(socket_t clientSock, const std::string& nameIfNew);
    Subscriber* get_subscriber_for_socket(socket_t clientSock);
    void remove_subscriber_for_socket(socket_t clientSock);
    bool create_path(const std::string& path);

    void ispisiSvePretplate(socket_t adminSock);
};

#endif
