#ifndef SHREDIS_CURSOR_H
#define SHREDIS_CURSOR_H

#include <vector>
#include <string>
#include <memory>
#include "DataStore.h"

enum DataStoreRespCode: uint32_t {
    DSRC_SUCCESS = 0,
    DSRC_INVALID_INPUT = 1,
    DSRC_SERVER_ERROR = 2,
};

typedef struct {
    std::vector<std::pair<char*, int>> words;
} cursor_cmd;

typedef struct {
    DataStoreRespCode code;
    std::string s;
} cursor_resp;

class Cursor {
public:
    explicit Cursor(DataStore& ds);
    cursor_resp process_query(char* query, size_t len);
private:
    DataStore& datastore;
    void parse_query(char* query, size_t len, cursor_cmd& cmd);
    void evaluate_cmd(const cursor_cmd& cmd, cursor_resp& resp);
    static void get_error_response(cursor_resp& resp, const std::string& msg);
};

#endif //SHREDIS_CURSOR_H
