#include "AVLNode.h"

#include <utility>
#include <cassert>

AVLNode::AVLNode(std::string key, std::string value): key(std::move(key)), value(std::move(value)) {

}

size_t AVLNode::validate_tree() const {
    size_t left_height = this->left ? this->left->validate_tree() : 0;
    size_t right_height = this->right ? this->right->validate_tree() : 0;
    assert(this->height_diff == right_height - left_height);
    assert(std::abs(this->height_diff) <= 1);
    return 1 + std::max(left_height, right_height);
}
