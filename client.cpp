#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <netinet/in.h>
#include <string>
#include <iostream>

#include <map>
#include <string>

void sanity_check() {
    std::map<std::string, std::string> m;
    std::string key = "abcd"; std::string value = "efgh";
    m[key] = value;
    std::string key2 = "abcd";
    if (m.find(key2) == m.end()) printf("CRITICAL\n");
    else printf("map working correctly\n");
}

typedef struct {
    std::string s;
    uint32_t code;
} response;

[[noreturn]] void crash(const std::string& base) {
    std::cerr << base << ": " << strerror(errno) << std::endl;
    std::exit(1);
}

ssize_t write_query(int fd, const std::string& query) {
    size_t query_len = query.length();
    char msg_buf[4 + query_len];
    memcpy(msg_buf, &query_len, 4);
    memcpy(&msg_buf[4], query.c_str(), query_len);
    return write(fd, msg_buf, 4 + query_len);
}

void print_response(const response& resp) {
    std::cout << "response: " << resp.code << ": " << resp.s << std::endl;
}

response read_response(int fd) {
    uint32_t resp_len;
    if (read(fd, &resp_len, 4) < 0) crash("len: read()");
    char resp[resp_len + 1];
    if (read(fd, resp, resp_len) < 0) crash("body: read()");
    resp[resp_len] = '\0';

    uint32_t resp_code = *((uint32_t*)resp);
    return {.s = resp + 4, .code = resp_code};
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

    write_query(fd, "GET somekey");
    write_query(fd, "SET somekey somevalue");
    write_query(fd, "GET somekey");
    write_query(fd, "GET somekey");
    write_query(fd, "DEL somekey");
    write_query(fd, "GET somekey");

    print_response(read_response(fd));
    print_response(read_response(fd));
    print_response(read_response(fd));
    print_response(read_response(fd));
    print_response(read_response(fd));
    print_response(read_response(fd));

    close(fd);
    return 0;
}