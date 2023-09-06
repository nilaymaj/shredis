#ifndef SHREDIS_AVLNODE_H
#define SHREDIS_AVLNODE_H

#include <string>

class AVLNode {
public:
    std::string key;
    std::string value;
    AVLNode* left = nullptr;
    AVLNode* right = nullptr;
    int height_diff = 0;

    AVLNode(std::string key, std::string value);
    size_t validate_tree() const;
};


#endif //SHREDIS_AVLNODE_H
