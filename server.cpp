#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>


int32_t read_from(int fd, char* buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) return -1;
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

int32_t write_to(int fd, char* buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) return -1;
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

int32_t process_query(int fd) {
    // read size of query body
    char query_len_raw[4];
    if (read_from(fd, query_len_raw, 4)) {
        printf("read_from(): %s\n", strerror(errno));
        return -1;
    }
    // assuming here that this system is little-endian
    uint32_t query_len = 0;
    memcpy(&query_len, query_len_raw, 4);

    // read query body
    char query[query_len + 1];
    if (read_from(fd, query, query_len)) {
        printf("read_from(): %s\n", strerror(errno));
        return -1;
    }
    query[query_len] = '\0';
    printf("client says: %s\n", query);

    const char reply[] = "world";
    char resp_buf[4 + sizeof(reply)];
    size_t reply_len = strlen(reply);
    memcpy(resp_buf, &reply_len, 4);
    memcpy(&resp_buf[4], reply, reply_len);
    return write_to(fd, resp_buf, 4 + reply_len);
}

void process_conn(int connfd, struct sockaddr_in *client_addr) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        printf("read(): %s", strerror(errno));
        return;
    }
    printf("client says: %s\n", rbuf);

    int client_port = htons(client_addr->sin_port);
    char client_ip[INET_ADDRSTRLEN + 1] = {};
    inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), client_ip, INET_ADDRSTRLEN);

    char wbuf[64] = {};
    sprintf(wbuf, "your addr: %s:%d", client_ip, client_port);
    write(connfd, wbuf, strlen(wbuf));
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        printf("bind(): %s", strerror(errno));
        exit(1);
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        printf("listen(): %s", strerror(errno));
        exit(1);
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr*)&client_addr, &socklen);
        if (connfd < 0) {
            printf("accept(): %s", strerror(errno));
            continue;
        }

        while (true) {
            int32_t err = process_query(connfd);
            if (err) break;
        }

        close(connfd);
    }

    return 0;
}