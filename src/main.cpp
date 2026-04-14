#include "network/broker.h"
#include <iostream>

int main(int argc, char** argv) {
    int port = 5555;
    if (argc >= 2) port = std::atoi(argv[1]);

    Broker b;
    b.run(port);
    return 0;
}
