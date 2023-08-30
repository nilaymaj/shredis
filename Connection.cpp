#include <stdexcept>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "Connection.h"

Connection::Connection(int fd): fd(fd) {}

void Connection::process() {
    if (this->state == STATE_READ) this->process_read();
    else if (this->state == STATE_WRITE) this->process_write();
    else throw std::logic_error("process() called on dead connection");
}

void Connection::process_read() {
    while (this->try_fill_read_buffer());
    while (this->try_process_query());
    this->clear_read_buffer_space();
    if (this->state == STATE_READ) this->state = STATE_WRITE;
}

void Connection::process_write() {
    while (this->try_flush_write_buffer());
    if (this->state == STATE_WRITE) this->state = STATE_READ;
}

bool Connection::try_fill_read_buffer() {
    // Read buffer full - process some queries first
    if (this->rbuf_size == sizeof(this->rbuf)) return false;

    // TODO: Handle if next query is longer than allowed

    // Read as much as possible from fd into read buffer
    size_t space_left = sizeof(this->rbuf) - this->rbuf_size;
    ssize_t rv = read(this->fd, &this->rbuf[this->rbuf_size], space_left);
    if (rv < 0) {
        if (errno == EAGAIN) return false;
        std::cout << "aborting conn: read(): " << strerror(errno) << std::endl;
        this->state = STATE_END;
        return false;
    }

    if (rv == 0) {
        this->state = STATE_END;
        return false;
    }

    this->rbuf_size += (size_t)rv;
    assert(this->rbuf_size <= sizeof(this->rbuf));
    return true;
}

bool Connection::try_process_query() {
    if (this->rbuf_size - this->rbuf_processed < 4) return false;

    // Read query body
    uint32_t request_len = 0;
    memcpy(&request_len, &this->rbuf[this->rbuf_processed], 4);
    if (request_len > k_max_msg) {
        std::cout << "request body too long (" << request_len << " bytes)" << std::endl;
        this->state = STATE_END;
        return false;
    }
    if (this->rbuf_size - this->rbuf_processed < 4 + request_len) return false;

    // If there's not enough space in write buffer, send to write flush state
    size_t wbuf_available_space = sizeof(this->wbuf) - this->wbuf_size;
    if (wbuf_available_space < 4 + request_len) {
        std::cout << "\x1b[31bnot enough space in wbuf, sending to STATE_WRITE\x1b[0m" << std::endl;
        this->state = STATE_WRITE;
        return false;
    }

    // Process query and write reply to write buffer
    printf("%d: client says: %.*s\n", this->fd, request_len, &rbuf[this->rbuf_processed + 4]);
    memcpy(&this->wbuf[this->wbuf_size], &request_len, 4);
    memcpy(&this->wbuf[this->wbuf_size + 4], &rbuf[this->rbuf_processed + 4], request_len);
    this->wbuf_size += 4 + request_len;
    this->rbuf_processed += 4 + request_len;

    return true;
}

void Connection::clear_read_buffer_space() {
    size_t remaining_bytes = sizeof(this->rbuf) - this->rbuf_processed;
    memmove(this->rbuf, &this->rbuf[this->rbuf_processed], remaining_bytes);
    this->rbuf_size -= this->rbuf_processed;
    this->rbuf_processed = 0;
}

bool Connection::try_flush_write_buffer() {
    // Write remaining bytes to buffer
    size_t remaining_bytes = this->wbuf_size - this->wbuf_sent;
    ssize_t rv = write(this->fd, &this->wbuf[this->wbuf_sent], remaining_bytes);

    if (rv < 0) {
        if (errno == EAGAIN) return false;
        std::cout << "aborting conn: write(): " << strerror(errno) << std::endl;
        this->state = STATE_END;
        return false;
    }

    this->wbuf_sent += rv;
    assert(this->wbuf_sent <= this->wbuf_size);

    // All done - move Connection back to req state
    if (this->wbuf_sent == this->wbuf_size) {
        this->state = STATE_READ;
        this->wbuf_sent = 0;
        this->wbuf_size = 0;
        return false;
    };

    // Bytes still remaining to be sent in write buffer
    return true;
}
