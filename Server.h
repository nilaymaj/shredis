#ifndef SHREDIS_SERVER_H
#define SHREDIS_SERVER_H


#include <vector>
#include "Connection.h"

class Server {
public:
    explicit Server();
    ~Server();
    void setup_listening_fd();
    [[noreturn]] void start_event_loop();

private:
    int listen_fd = -1;
    std::vector<Connection*> fd2conn;

    static void fd_set_nonblock(int fd);
    void accept_client_connection();
};


#endif //SHREDIS_SERVER_H
