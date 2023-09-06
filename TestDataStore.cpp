#include <cassert>
#include <vector>
#include <iostream>
#include "DataStore.h"

std::vector<std::string> base_keys {
    "10", "05", "15", "02", "03", "08", "07", "06",
    "12", "13", "17", "18", "09",
};

int main() {
    DataStore ds;
    assert(ds.get("somekey") == "key does not exist");
    for (auto& key : base_keys) {
        ds.set(key, "value" + key);
        ds.validate();
    }
    for (auto& key : base_keys) assert(ds.get(key) == "value" + key);
}