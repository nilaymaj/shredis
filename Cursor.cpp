#include <cstring>
#include <iostream>
#include "Cursor.h"
#include "Common.h"

bool match_word(char* word, const std::string& target, size_t word_len) {
    if (word_len != target.length()) return false;
    return std::strncmp(word, target.c_str(), word_len) == 0;
}

Cursor::Cursor(DataStore& ds): datastore(ds) {};

cursor_resp Cursor::process_query(char *query, const size_t len) {
    cursor_cmd cmd = {};
    cursor_resp resp = {};
    parse_query(query, len, cmd);
    this->evaluate_cmd(cmd, resp);
    return resp;
}

void Cursor::parse_query(char *query, const size_t len, cursor_cmd &cmd) {
    char* curr_start = query;
    for (int i = 0; i < len; ++i) {
        if (query[i] == ' ' || query[i] == '\0') {
            long word_len = query + i - curr_start;
            cmd.words.emplace_back(curr_start, word_len);
            curr_start = query + i + 1;
        }
    }
    cmd.words.emplace_back(curr_start, query + len - curr_start);
}

void Cursor::evaluate_cmd(const cursor_cmd &cmd, cursor_resp &resp) {
    if (cmd.words.empty()) {
        Cursor::get_error_response(resp, "empty query");
        return;
    }

    if (match_word(cmd.words[0].first, "GET", cmd.words[0].second)) {
        if (cmd.words.size() != 2) {
            Cursor::get_error_response(resp, "get query must have 2 words");
            return;
        }
        std::string key(cmd.words[1].first, cmd.words[1].second);
        resp.s = this->datastore.get(key);
        resp.code = DSRC_SUCCESS;
        return;
    } else if (match_word(cmd.words[0].first, "SET", cmd.words[0].second)) {
        if (cmd.words.size() != 3) {
            Cursor::get_error_response(resp, "set query must have 3 words");
            return;
        }
        std::string key(cmd.words[1].first, cmd.words[1].second);
        std::string value(cmd.words[2].first, cmd.words[2].second);
        this->datastore.set(key, value);
        resp.s = "OK";
        resp.code = DSRC_SUCCESS;
        return;
    } else if (match_word(cmd.words[0].first, "DEL", cmd.words[0].second)) {
        if (cmd.words.size() != 2) {
            Cursor::get_error_response(resp, "del query must have 2 words");
            return;
        }
        std::string key(cmd.words[1].first, cmd.words[1].second);
        this->datastore.del(key);
        resp.s = "OK";
        resp.code = DSRC_SUCCESS;
        return;
    }

    resp.s = "unknown command";
    resp.code = DSRC_INVALID_INPUT;
}

void Cursor::get_error_response(cursor_resp &resp, const std::string& msg) {
    resp.code = DSRC_INVALID_INPUT;
    resp.s = "failed to process query: " + msg;
}
