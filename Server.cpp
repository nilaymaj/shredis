#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <cstring>
#include "Server.h"

[[noreturn]] void crash(const std::string& base) {
    std::cerr << base << ": " << strerror(errno) << std::endl;
    std::exit(1);
}

Server::Server() {
    this->epoll_fd = epoll_create(1);
    if (this->epoll_fd < 0) crash("epoll_create()");
};

Server::~Server() {
    if (close(this->epoll_fd))
        std::cout << "failed to close epoll fd: " << strerror(errno) << std::endl;
};

void Server::setup_listening_fd() {
    std::cout << "Setting up listening fd... ";
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) crash("socket()");

    // Set SO_REUSEADDR for listening socket
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // Bind to local address on port 1234
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) crash("bind()");

    rv = listen(fd, SOMAXCONN);
    if (rv) crash("listen()");

    Server::fd_set_nonblock(fd);

    struct epoll_event ee = {.events = EPOLLIN, .data = {.fd = fd}};
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &ee))
        crash("epoll_ctl(): failed for listen fd");

    this->listen_fd = fd;
    std::cout << "\x1b[32m success\x1b[0m" << std::endl;
}

void Server::start_event_loop() {
    std::cout << "\x1b[34mStarted event loop!\x1b[0m" << std::endl;

    while (true) {
        // Update subscribed events for all existing connections
        for (Connection* conn : fd2conn) {
            if (!conn) continue;
            struct epoll_event ee = {.events = 0, .data = {.fd = conn->fd}};
            ee.events = (conn->state == STATE_WRITE) ? EPOLLOUT : EPOLLIN;
            ee.events = ee.events | EPOLLERR;
            if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, conn->fd, &ee)) {
                printf("%d: epoll_ctl: mod: %s\n", conn->fd, strerror(errno));
                conn->state = STATE_END;
            };
        }

        // Wait for events on current connections
        struct epoll_event ready_events[16];
        int ready_count = epoll_wait(this->epoll_fd, ready_events, sizeof(ready_events), 5000);
        if (ready_count < 0) {
            printf("epoll_wait(): %s\n", strerror(errno));
            continue;
        };

        // Process ready connections
        for (size_t i = 0; i < ready_count; ++i) {
            if (ready_events[i].data.fd != this->listen_fd) {
                Connection *conn = this->fd2conn[ready_events[i].data.fd];
                if (conn->state == STATE_END) this->close_client_connection(conn);
                else conn->process();
            } else accept_client_connection();
        }
    }
}

bool Server::fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) crash("fcntl(): get");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        printf("%d: fcntl: %s\n", fd, strerror(errno));
        return false;
    } else return true;
}

void Server::accept_client_connection() {
    // Accept client Connection and configure fd
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int client_fd = accept(this->listen_fd, (struct sockaddr*)&client_addr, &socklen);
    if (client_fd < 0) crash("accept()");
    if (!Server::fd_set_nonblock(client_fd)) {
        close(client_fd);
        return;
    };

    // Create Connection instance and insert into conn vector
    auto conn = new Connection(client_fd, this->datastore);
    if (this->fd2conn.size() <= client_fd) this->fd2conn.resize(client_fd + 1);
    this->fd2conn[client_fd] = conn;

    struct epoll_event ee = {.events = 0, .data = {.fd = conn->fd}};
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &ee)) {
        printf("%d: epoll_ctl: add: %s\n", client_fd, strerror(errno));
        conn->state = STATE_END;
    };
}

void Server::close_client_connection(Connection* conn) {
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, conn->fd, nullptr))
        printf("%d: epoll_ctl[del]: %s\n", conn->fd, strerror(errno));
    fd2conn[conn->fd] = nullptr;
    close(conn->fd);
    free(conn);
}