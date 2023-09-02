#ifndef SHREDIS_DATASTORE_H
#define SHREDIS_DATASTORE_H

#include <string>
#include <map>

class DataStore {
public:
    DataStore();
    std::string get(const std::string& key);
    void set(const std::string& key, const std::string& value);
    int del(const std::string& key);
private:
    std::map<std::string, std::string> mapper;
};

#endif //SHREDIS_DATASTORE_H
