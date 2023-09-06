#include <iostream>
#include <stack>
#include "DataStore.h"
#include "Common.h"

std::string DataStore::get(const std::string& key) {
    get_node_result result = this->get_node(key, nullptr);
    return result.exists ? result.node->value : "key does not exist";
}

void DataStore::set(const std::string& key, const std::string& value) {
    get_node_result result = this->get_node(key, nullptr);
    if (result.exists) {
        result.node->value = value;
        return;
    } else {
        bool insert_result = this->insert_node(key, value, true);
        if (!insert_result) throw std::logic_error("unexpected failure in insert_node");
    }
}

int DataStore::del(const std::string& key) {
    if (this->mapper.find(key) == this->mapper.end()) return 1;
    this->mapper.erase(key);
    return 0;
}

DataStore::DataStore() {
    std::cout << "initializing datastore" << std::endl;
}

AVLNode *DataStore::rotate_left(AVLNode *node) {
    if (node->right == nullptr) {
        debug("warn: rotate_left called on invalid node");
        return node;
    }

    AVLNode* right_child = node->right;
    node->right = right_child->left;
    right_child->left = node;

    node->height_diff -= 1 + std::max(right_child->height_diff, 0);
    right_child->height_diff -= 1 - std::min(node->height_diff, 0);
    return right_child;
}

AVLNode *DataStore::rotate_right(AVLNode *node) {
    if (node->left == nullptr) {
        debug("warn: rotate_right called on invalid node");
        return node;
    }
    AVLNode* left_child = node->left;
    node->left = left_child->right;
    left_child->right = node;

    node->height_diff += 1 - std::min(left_child->height_diff, 0);
    left_child->height_diff += 1 + std::max(node->height_diff, 0);
    return left_child;
}

AVLNode* DataStore::fix_node_after_operation(AVLNode* node, const std::string& key) {
    AVLNode* z = node;
    AVLNode* y = (key < z->key) ? z->left : z->right;
    AVLNode* x = (key < y->key) ? y->left : y->right;

    if (y == z->left && x == y->left) return this->rotate_right(z);
    else if (y == z->right && x == y->right) return this->rotate_left(z);
    else if (y == z->left && x == y->right) {
        z->left = this->rotate_left(y);
        return this->rotate_right(z);
    } else if (y == z->right && x == y->left) {
        z->right = this->rotate_left(y);
        return this->rotate_right(z);
    } else throw std::logic_error("unknown condition in fixing avl node");
}

bool DataStore::insert_node(const std::string &key, const std::string& value, bool upsert) {
    if (this->root == nullptr) {
        this->root = new AVLNode(key, value);
        return true;
    }

    // Find appropriate node or parent
    std::stack<AVLNode*> root_path;
    auto result = this->get_node(key, &root_path);
    if (result.exists) {
        if (!upsert) return false;
        result.node->value = value;
        return true;
    };

    // Insert new node at correct position
    auto new_node = new AVLNode(key, value);
    if (key < result.node->key) result.node->left = new_node;
    else result.node->right = new_node;

    // Bubble up from new node and update height diff until either:
    // - an unbalanced node is reached: fix the imbalance
    // - a node is reached whose subtree height is not changed by operation
    AVLNode* curr_node = new_node;
    while (!root_path.empty()) {
        AVLNode* parent = root_path.top(); root_path.pop();
        parent->height_diff += (parent->right == curr_node) ? 1 : -1;
        if (parent->height_diff == 0) break;
        else if (std::abs(parent->height_diff) == 2) {
            AVLNode* gp = root_path.empty() ? nullptr : root_path.top();
            AVLNode** new_subtree_root_ptr = &this->root;
            if (gp && gp->left == parent) new_subtree_root_ptr = &gp->left;
            else if (gp && gp->right == parent) new_subtree_root_ptr = &gp->right;
            *new_subtree_root_ptr = this->fix_node_after_operation(parent, key);
            break;
        } else curr_node = parent;
    }

    return true;
}

get_node_result DataStore::get_node(const std::string &key, std::stack<AVLNode*> *stack) {
    AVLNode* curr = this->root;
    if (this->root == nullptr) return {.node = nullptr, .exists = false};

    while (true) {
        if (stack != nullptr) stack->push(curr);
        auto diff = key.compare(curr->key);
        if (diff == 0) return {.node = curr, .exists = true};
        auto next_node = diff < 0 ? curr->left : curr->right;
        if (next_node == nullptr) return {.node = curr, .exists = false};
        curr = next_node;
    }
}

bool DataStore::validate() {
    return this->root->validate_tree();
}
