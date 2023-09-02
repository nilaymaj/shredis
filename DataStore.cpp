#include <iostream>
#include "DataStore.h"

std::string DataStore::get(const std::string& key) {
    auto it = this->mapper.find(key);
    return (it == this->mapper.end()) ? "key does not exist" : it->second;
}

void DataStore::set(const std::string& key, const std::string& value) {
    this->mapper[key] = value;
}

int DataStore::del(const std::string& key) {
    if (this->mapper.find(key) == this->mapper.end()) return 1;
    this->mapper.erase(key);
    return 0;
}

DataStore::DataStore() {
    std::cout << "initializing datastore" << std::endl;
}
