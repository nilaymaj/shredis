#ifndef SHREDIS_CONNECTION_H
#define SHREDIS_CONNECTION_H


#include <cstddef>
#include <cstdint>

const size_t k_max_msg = 56;

enum ConnectionState {
    STATE_READ = 0,
    STATE_WRITE = 1,
    STATE_END = 2,
};

class Connection {
public:
    int fd = -1;
    ConnectionState state = STATE_READ;

    explicit Connection(int fd);
    void process();
private:
    size_t rbuf_size = 0;
    uint8_t rbuf[4 + k_max_msg] = {};
    size_t wbuf_size = 0;
    size_t wbuf_sent = 0;
    uint8_t wbuf[4 + k_max_msg] = {};

    void process_read();
    bool try_fill_read_buffer();
    bool try_process_query();

    void process_write();
    bool try_flush_write_buffer();
};


#endif //SHREDIS_CONNECTION_H
