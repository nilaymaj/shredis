#ifndef SHREDIS_DATASTORE_H
#define SHREDIS_DATASTORE_H

#include <string>
#include <stack>
#include <map>
#include "AVLNode.h"

typedef struct {
    AVLNode* node;
    bool exists;
} get_node_result;

class DataStore {
public:
    DataStore();
    std::string get(const std::string& key);
    void set(const std::string& key, const std::string& value);
    int del(const std::string& key);
    bool validate();
private:
    std::map<std::string, std::string> mapper;
    AVLNode* root = nullptr;

    AVLNode* rotate_left(AVLNode* node);
    AVLNode* rotate_right(AVLNode* node);

    get_node_result get_node(const std::string& key, std::stack<AVLNode*>* st);
    bool insert_node(const std::string& key, const std::string& value, bool upsert);
    AVLNode *fix_node_after_operation(AVLNode *node, const std::string &key);
};

#endif //SHREDIS_DATASTORE_H
