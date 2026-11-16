#pragma once

#include <cstddef>
#include <utility>
#include <stdexcept>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief B+ Tree Data Structure (Order M).
 * 
 * High-fanout balanced search tree designed for indexed file/memory stores.
 * All values reside in leaf nodes, which are linked sequentially for O(k) range scans.
 * 
 * Time Complexities:
 *  - Search: O(log_M N)
 *  - Insert: O(M * log_M N)
 *  - Range Query [low, high]: O(log_M N + k)
 * 
 * Space Complexity: O(N).
 */
template <typename Key, typename Value, size_t ORDER = 4>
class BPlusTree {
public:
    static_assert(ORDER >= 3, "B+ Tree ORDER must be at least 3");

    struct Node {
        bool isLeaf;
        size_t numKeys;
        Key keys[ORDER];
        Node* children[ORDER + 1]; // For internal nodes: child pointers
        Value values[ORDER];       // For leaf nodes: stored values
        Node* next;                // For leaf nodes: linked list pointer
        Node* prev;                // For leaf nodes: backwards pointer

        explicit Node(bool leaf = false)
            : isLeaf(leaf), numKeys(0), next(nullptr), prev(nullptr) {
            for (size_t i = 0; i <= ORDER; ++i) {
                children[i] = nullptr;
            }
        }
    };

private:
    Node* root_;
    Node* leafHead_; // Pointer to first leaf for full sequential scans
    size_t size_;

    Node* findLeaf(const Key& key) const {
        if (!root_) return nullptr;
        Node* curr = root_;
        while (!curr->isLeaf) {
            size_t i = 0;
            while (i < curr->numKeys && key >= curr->keys[i]) {
                ++i;
            }
            curr = curr->children[i];
        }
        return curr;
    }

    void insertInternal(const Key& key, Node* parent, Node* child) {
        if (parent->numKeys < ORDER) {
            // Room available in parent node
            size_t i = 0;
            while (i < parent->numKeys && parent->keys[i] < key) {
                ++i;
            }
            for (size_t j = parent->numKeys; j > i; --j) {
                parent->keys[j] = parent->keys[j - 1];
                parent->children[j + 1] = parent->children[j];
            }
            parent->keys[i] = key;
            parent->children[i + 1] = child;
            ++parent->numKeys;
        } else {
            // Parent is full, must split parent
            Node* newInternal = new Node(false);
            Key virtualKeys[ORDER + 1];
            Node* virtualChildren[ORDER + 2];

            size_t i = 0;
            while (i < ORDER && parent->keys[i] < key) {
                virtualKeys[i] = parent->keys[i];
                virtualChildren[i] = parent->children[i];
                ++i;
            }
            virtualKeys[i] = key;
            virtualChildren[i] = parent->children[i];
            virtualChildren[i + 1] = child;

            for (size_t j = i + 1; j <= ORDER; ++j) {
                virtualKeys[j] = parent->keys[j - 1];
                virtualChildren[j + 1] = parent->children[j];
            }

            size_t splitIdx = (ORDER + 1) / 2;
            Key promotedKey = virtualKeys[splitIdx];

            parent->numKeys = splitIdx;
            for (size_t j = 0; j < splitIdx; ++j) {
                parent->keys[j] = virtualKeys[j];
                parent->children[j] = virtualChildren[j];
            }
            parent->children[splitIdx] = virtualChildren[splitIdx];

            newInternal->numKeys = ORDER - splitIdx;
            for (size_t j = 0; j < newInternal->numKeys; ++j) {
                newInternal->keys[j] = virtualKeys[splitIdx + 1 + j];
                newInternal->children[j] = virtualChildren[splitIdx + 1 + j];
            }
            newInternal->children[newInternal->numKeys] = virtualChildren[ORDER + 1];

            if (parent == root_) {
                Node* newRoot = new Node(false);
                newRoot->keys[0] = promotedKey;
                newRoot->children[0] = parent;
                newRoot->children[1] = newInternal;
                newRoot->numKeys = 1;
                root_ = newRoot;
            } else {
                Node* grandParent = getParent(root_, parent);
                insertInternal(promotedKey, grandParent, newInternal);
            }
        }
    }

    Node* getParent(Node* curr, Node* child) {
        if (!curr || curr->isLeaf) return nullptr;
        for (size_t i = 0; i <= curr->numKeys; ++i) {
            if (curr->children[i] == child) return curr;
            Node* p = getParent(curr->children[i], child);
            if (p) return p;
        }
        return nullptr;
    }

    void freeSubtree(Node* n) {
        if (!n) return;
        if (!n->isLeaf) {
            for (size_t i = 0; i <= n->numKeys; ++i) {
                freeSubtree(n->children[i]);
            }
        }
        delete n;
    }

public:
    BPlusTree() : root_(nullptr), leafHead_(nullptr), size_(0) {}

    ~BPlusTree() {
        clear();
    }

    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void clear() {
        freeSubtree(root_);
        root_ = nullptr;
        leafHead_ = nullptr;
        size_ = 0;
    }

    // Search by key - O(log_M N)
    Value* find(const Key& key) {
        Node* leaf = findLeaf(key);
        if (!leaf) return nullptr;

        for (size_t i = 0; i < leaf->numKeys; ++i) {
            if (leaf->keys[i] == key) {
                return &(leaf->values[i]);
            }
        }
        return nullptr;
    }

    const Value* find(const Key& key) const {
        Node* leaf = findLeaf(key);
        if (!leaf) return nullptr;

        for (size_t i = 0; i < leaf->numKeys; ++i) {
            if (leaf->keys[i] == key) {
                return &(leaf->values[i]);
            }
        }
        return nullptr;
    }

    // Insert key-value pair - O(M * log_M N)
    bool insert(const Key& key, const Value& value) {
        if (!root_) {
            root_ = new Node(true);
            root_->keys[0] = key;
            root_->values[0] = value;
            root_->numKeys = 1;
            leafHead_ = root_;
            ++size_;
            return true;
        }

        Node* leaf = findLeaf(key);
        // Check if key already exists (update)
        for (size_t i = 0; i < leaf->numKeys; ++i) {
            if (leaf->keys[i] == key) {
                leaf->values[i] = value;
                return false;
            }
        }

        if (leaf->numKeys < ORDER) {
            // Room in leaf node
            size_t i = 0;
            while (i < leaf->numKeys && leaf->keys[i] < key) {
                ++i;
            }
            for (size_t j = leaf->numKeys; j > i; --j) {
                leaf->keys[j] = leaf->keys[j - 1];
                leaf->values[j] = leaf->values[j - 1];
            }
            leaf->keys[i] = key;
            leaf->values[i] = value;
            ++leaf->numKeys;
            ++size_;
            return true;
        }

        // Leaf is full, split leaf
        Node* newLeaf = new Node(true);
        Key virtualKeys[ORDER + 1];
        Value virtualValues[ORDER + 1];

        size_t i = 0;
        while (i < ORDER && leaf->keys[i] < key) {
            virtualKeys[i] = leaf->keys[i];
            virtualValues[i] = leaf->values[i];
            ++i;
        }
        virtualKeys[i] = key;
        virtualValues[i] = value;
        for (size_t j = i + 1; j <= ORDER; ++j) {
            virtualKeys[j] = leaf->keys[j - 1];
            virtualValues[j] = leaf->values[j - 1];
        }

        size_t splitIdx = (ORDER + 1) / 2;
        leaf->numKeys = splitIdx;
        for (size_t j = 0; j < splitIdx; ++j) {
            leaf->keys[j] = virtualKeys[j];
            leaf->values[j] = virtualValues[j];
        }

        newLeaf->numKeys = (ORDER + 1) - splitIdx;
        for (size_t j = 0; j < newLeaf->numKeys; ++j) {
            newLeaf->keys[j] = virtualKeys[splitIdx + j];
            newLeaf->values[j] = virtualValues[splitIdx + j];
        }

        // Maintain linked list pointers between leaves
        newLeaf->next = leaf->next;
        if (leaf->next) {
            leaf->next->prev = newLeaf;
        }
        leaf->next = newLeaf;
        newLeaf->prev = leaf;

        if (leaf == root_) {
            Node* newRoot = new Node(false);
            newRoot->keys[0] = newLeaf->keys[0];
            newRoot->children[0] = leaf;
            newRoot->children[1] = newLeaf;
            newRoot->numKeys = 1;
            root_ = newRoot;
        } else {
            Node* parent = getParent(root_, leaf);
            insertInternal(newLeaf->keys[0], parent, newLeaf);
        }

        ++size_;
        return true;
    }

    // Range search [low, high] utilizing leaf linked list - O(log_M N + k)
    Vector<std::pair<Key, Value>> findRange(const Key& low, const Key& high) const {
        Vector<std::pair<Key, Value>> result;
        Node* leaf = findLeaf(low);
        if (!leaf) return result;

        bool done = false;
        while (leaf && !done) {
            for (size_t i = 0; i < leaf->numKeys; ++i) {
                if (leaf->keys[i] >= low && leaf->keys[i] <= high) {
                    result.push_back({leaf->keys[i], leaf->values[i]});
                } else if (leaf->keys[i] > high) {
                    done = true;
                    break;
                }
            }
            leaf = leaf->next;
        }
        return result;
    }
};

} // namespace dsa
