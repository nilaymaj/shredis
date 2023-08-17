#include <iostream>
#include "Server.h"

int main() {
    std::cout << "Initializing server..." << std::endl;
    auto server = Server();
    server.setup_listening_fd();
    server.start_event_loop();
}
