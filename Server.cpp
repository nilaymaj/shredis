#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <cstring>
#include "Server.h"

void crash(const std::string& base) {
    std::cerr << base << ": " << strerror(errno) << std::endl;
    std::exit(1);
}

Server::Server() = default;

Server::~Server() = default;

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

    this->listen_fd = fd;
    std::cout << "\x1b[32m success\x1b[0m" << std::endl;
}

void Server::start_event_loop() {
    std::vector<struct pollfd> poll_args;
    std::cout << "\x1b[34mStarted event loop!\x1b[0m" << std::endl;

    while (true) {
        poll_args.clear();
        poll_args.push_back({this->listen_fd, POLLIN, 0});

        for (Connection* conn : fd2conn) {
            if (!conn) continue;
            struct pollfd pfd = {conn->fd};
            pfd.events = (conn->state == STATE_WRITE) ? POLLOUT : POLLIN;
            pfd.events = pfd.events | POLLERR;
            poll_args.push_back(pfd);
        }

        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
        if (rv < 0) crash("poll()");

        for (size_t i = 1; i < poll_args.size(); ++i) {
            if (!poll_args[i].revents) continue;
            Connection* conn = this->fd2conn[poll_args[i].fd];
            if (conn->state == STATE_END) {
                fd2conn[conn->fd] = nullptr;
                close(conn->fd);
                free(conn);
            } else conn->process();
        }

        if (poll_args[0].revents) accept_client_connection();
    }
}

void Server::fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) crash("fcntl(): get");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) crash("fcntl(): set");
}

void Server::accept_client_connection() {
    // Accept client Connection and configure fd
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int client_fd = accept(this->listen_fd, (struct sockaddr*)&client_addr, &socklen);
    if (client_fd < 0) crash("accept()");
    Server::fd_set_nonblock(client_fd);

    // Create Connection instance and insert into conn vector
    auto conn = new Connection(client_fd);
    if (this->fd2conn.size() <= client_fd) this->fd2conn.resize(client_fd + 1);
    this->fd2conn[client_fd] = conn;
}
