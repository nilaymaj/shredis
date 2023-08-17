#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <netinet/in.h>
#include <string>

ssize_t write_query(int fd, const std::string& query) {
    size_t query_len = query.length() + 1;
    char msg_buf[4 + query_len];
    memcpy(msg_buf, &query_len, 4);
    memcpy(&msg_buf[4], query.c_str(), query_len);
    return write(fd, msg_buf, 4 + query_len);
}

std::string read_response(int fd) {
    uint32_t resp_len;
    if (read(fd, &resp_len, 4) < 0) {
        printf("len: read(): %s\n", strerror(errno));
        return "";
    };

    char resp[resp_len + 1];
    if (read(fd, resp, resp_len) < 0) {
        printf("body: read(): %s\n", strerror(errno));
        return "";
    };
    resp[resp_len] = '\0';

    return {resp, resp_len + 1};
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("socket(): %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if (rv) {
        printf("connect(): %s\n", strerror(errno));
        exit(1);
    }

    write_query(fd, "hello");
    write_query(fd, "another hello");
    write_query(fd, "another another hello");
    write_query(fd, "hello");
    write_query(fd, "another hello");
    write_query(fd, "another another hello");

    printf("response: %s\n", read_response(fd).c_str());
    printf("response: %s\n", read_response(fd).c_str());
    printf("response: %s\n", read_response(fd).c_str());
    printf("response: %s\n", read_response(fd).c_str());
    printf("response: %s\n", read_response(fd).c_str());
    printf("response: %s\n", read_response(fd).c_str());

    close(fd);
    return 0;
}