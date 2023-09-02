#ifndef SHREDIS_SERVER_H
#define SHREDIS_SERVER_H


#include <vector>
#include <sys/epoll.h>
#include "Connection.h"
#include "DataStore.h"

class Server {
public:
    Server();
    ~Server();
    void setup_listening_fd();
    [[noreturn]] void start_event_loop();

private:
    int listen_fd = -1;
    int epoll_fd = -1;
    std::vector<Connection*> fd2conn;
    DataStore datastore;

    static bool fd_set_nonblock(int fd);
    void accept_client_connection();
    void close_client_connection(Connection *conn);
};


#endif //SHREDIS_SERVER_H
