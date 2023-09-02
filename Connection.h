#ifndef SHREDIS_CONNECTION_H
#define SHREDIS_CONNECTION_H


#include <cstddef>
#include <cstdint>
#include "Cursor.h"

const size_t k_max_msg = 192;

enum ConnectionState {
    STATE_READ = 0,
    STATE_WRITE = 1,
    STATE_END = 2,
};

class Connection {
public:
    int fd = -1;
    ConnectionState state = STATE_READ;

    explicit Connection(int fd, DataStore& ds);
    void process();
private:
    Cursor cursor;
    size_t rbuf_size = 0;
    uint8_t rbuf[4 + k_max_msg] = {};
    size_t rbuf_processed = 0;
    size_t wbuf_size = 0;
    size_t wbuf_sent = 0;
    uint8_t wbuf[4 + k_max_msg] = {};

    void process_read();
    bool try_fill_read_buffer();
    bool try_process_query();
    void clear_read_buffer_space();

    void process_write();
    bool try_flush_write_buffer();
};


#endif //SHREDIS_CONNECTION_H
