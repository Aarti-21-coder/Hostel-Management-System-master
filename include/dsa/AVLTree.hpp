#pragma once

#include <cstddef>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Self-Balancing AVL Tree.
 * 
 * Used for fast range queries on student CGPA, age, and priority scores.
 * 
 * Time Complexities:
 *  - Search: O(log N)
 *  - Insert: O(log N)
 *  - Erase: O(log N)
 *  - Range Query [low, high]: O(log N + k), where k is number of items in range
 * 
 * Space Complexity: O(N).
 */
template <typename Key, typename Value, typename Compare = std::less<Key>>
class AVLTree {
public:
    struct Node {
        Key key;
        Value value;
        int height;
        Node* left;
        Node* right;

        Node(const Key& k, const Value& v)
            : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
    };

private:
    Node* root_;
    size_t size_;
    Compare comp_;

    int height(Node* n) const noexcept {
        return n ? n->height : 0;
    }

    int balanceFactor(Node* n) const noexcept {
        return n ? (height(n->left) - height(n->right)) : 0;
    }

    void updateHeight(Node* n) noexcept {
        if (n) {
            n->height = 1 + std::max(height(n->left), height(n->right));
        }
    }

    // Right Rotation (LL Case)
    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    // Left Rotation (RR Case)
    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    Node* insertInternal(Node* node, const Key& key, const Value& value, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key, value);
        }

        if (comp_(key, node->key)) {
            node->left = insertInternal(node->left, key, value, inserted);
        } else if (comp_(node->key, key)) {
            node->right = insertInternal(node->right, key, value, inserted);
        } else {
            // Key already exists, update value
            node->value = value;
            inserted = false;
            return node;
        }

        updateHeight(node);
        int balance = balanceFactor(node);

        // LL Case
        if (balance > 1 && comp_(key, node->left->key)) {
            return rotateRight(node);
        }
        // RR Case
        if (balance < -1 && comp_(node->right->key, key)) {
            return rotateLeft(node);
        }
        // LR Case
        if (balance > 1 && comp_(node->left->key, key)) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        // RL Case
        if (balance < -1 && comp_(key, node->right->key)) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    Node* minValueNode(Node* node) {
        Node* curr = node;
        while (curr && curr->left) {
            curr = curr->left;
        }
        return curr;
    }

    Node* eraseInternal(Node* node, const Key& key, bool& removed) {
        if (!node) return nullptr;

        if (comp_(key, node->key)) {
            node->left = eraseInternal(node->left, key, removed);
        } else if (comp_(node->key, key)) {
            node->right = eraseInternal(node->right, key, removed);
        } else {
            removed = true;
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) {
                    temp = node;
                    node = nullptr;
                } else {
                    *node = *temp;
                }
                delete temp;
            } else {
                Node* temp = minValueNode(node->right);
                node->key = temp->key;
                node->value = temp->value;
                node->right = eraseInternal(node->right, temp->key, removed);
            }
        }

        if (!node) return nullptr;

        updateHeight(node);
        int balance = balanceFactor(node);

        // LL Case
        if (balance > 1 && balanceFactor(node->left) >= 0) {
            return rotateRight(node);
        }
        // LR Case
        if (balance > 1 && balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        // RR Case
        if (balance < -1 && balanceFactor(node->right) <= 0) {
            return rotateLeft(node);
        }
        // RL Case
        if (balance < -1 && balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    void rangeQueryInternal(Node* node, const Key& low, const Key& high,
                            Vector<std::pair<Key, Value>>& result) const {
        if (!node) return;

        // If node key > low, there might be keys in the left subtree
        if (comp_(low, node->key)) {
            rangeQueryInternal(node->left, low, high, result);
        }

        // If low <= node->key <= high, include in result
        if (!comp_(node->key, low) && !comp_(high, node->key)) {
            result.push_back({node->key, node->value});
        }

        // If node key < high, there might be keys in the right subtree
        if (comp_(node->key, high)) {
            rangeQueryInternal(node->right, low, high, result);
        }
    }

    void freeAll(Node* node) {
        if (node) {
            freeAll(node->left);
            freeAll(node->right);
            delete node;
        }
    }

public:
    explicit AVLTree(Compare comp = Compare())
        : root_(nullptr), size_(0), comp_(comp) {}

    ~AVLTree() {
        freeAll(root_);
    }

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    AVLTree(AVLTree&& other) noexcept
        : root_(other.root_), size_(other.size_), comp_(other.comp_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            freeAll(root_);
            root_ = other.root_;
            size_ = other.size_;
            comp_ = other.comp_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] int height() const noexcept { return height(root_); }

    void clear() {
        freeAll(root_);
        root_ = nullptr;
        size_ = 0;
    }

    // Insert or update key-value pair - O(log N)
    bool insert(const Key& key, const Value& value) {
        bool inserted = false;
        root_ = insertInternal(root_, key, value, inserted);
        if (inserted) ++size_;
        return inserted;
    }

    // Search by key - O(log N)
    Value* find(const Key& key) {
        Node* curr = root_;
        while (curr) {
            if (comp_(key, curr->key)) {
                curr = curr->left;
            } else if (comp_(curr->key, key)) {
                curr = curr->right;
            } else {
                return &(curr->value);
            }
        }
        return nullptr;
    }

    const Value* find(const Key& key) const {
        Node* curr = root_;
        while (curr) {
            if (comp_(key, curr->key)) {
                curr = curr->left;
            } else if (comp_(curr->key, key)) {
                curr = curr->right;
            } else {
                return &(curr->value);
            }
        }
        return nullptr;
    }

    [[nodiscard]] bool contains(const Key& key) const {
        return find(key) != nullptr;
    }

    // Erase key - O(log N)
    bool erase(const Key& key) {
        bool removed = false;
        root_ = eraseInternal(root_, key, removed);
        if (removed) --size_;
        return removed;
    }

    // Range search [low, high] - O(log N + k)
    Vector<std::pair<Key, Value>> findRange(const Key& low, const Key& high) const {
        Vector<std::pair<Key, Value>> result;
        rangeQueryInternal(root_, low, high, result);
        return result;
    }
};

} // namespace dsa
