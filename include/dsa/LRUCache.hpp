#pragma once

#include <cstddef>
#include <utility>
#include "dsa/HashTable.hpp"
#include "dsa/DoublyLinkedList.hpp"

namespace dsa {

/**
 * @brief Least Recently Used (LRU) Cache.
 * 
 * Used for high-speed caching of recently viewed student and room profiles.
 * Combines a Hash Table with a Doubly Linked List.
 * 
 * Time Complexities:
 *  - Get: O(1) average
 *  - Put: O(1) average
 * 
 * Space Complexity: O(Capacity).
 */
template <typename Key, typename Value>
class LRUCache {
public:
    using CacheItem = std::pair<Key, Value>;
    using ListNode = typename DoublyLinkedList<CacheItem>::Node;

private:
    size_t capacity_;
    DoublyLinkedList<CacheItem> list_;
    HashTable<Key, ListNode*> map_;

public:
    explicit LRUCache(size_t capacity = 128) : capacity_(capacity) {
        if (capacity_ == 0) capacity_ = 1;
    }

    [[nodiscard]] size_t size() const noexcept { return list_.size(); }
    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return list_.empty(); }

    void clear() {
        list_.clear();
        map_.clear();
    }

    // Retrieve value by key and refresh recency - O(1)
    Value* get(const Key& key) {
        ListNode** nodePtr = map_.find(key);
        if (!nodePtr || !(*nodePtr)) {
            return nullptr; // Cache miss
        }

        ListNode* node = *nodePtr;
        // Move to front (most recently used)
        CacheItem item = std::move(node->data);
        list_.removeNode(node);
        ListNode* newHead = list_.push_front(std::move(item));
        map_.insert(key, newHead);

        return &(newHead->data.second);
    }

    // Insert or update value, evicting LRU if at capacity - O(1)
    void put(const Key& key, const Value& value) {
        ListNode** nodePtr = map_.find(key);
        if (nodePtr && (*nodePtr)) {
            // Key already exists, update and move to front
            ListNode* node = *nodePtr;
            list_.removeNode(node);
            ListNode* newHead = list_.push_front({key, value});
            map_.insert(key, newHead);
            return;
        }

        // Evict least recently used (tail) if full
        if (list_.size() >= capacity_) {
            ListNode* tail = list_.tail();
            if (tail) {
                map_.erase(tail->data.first);
                list_.pop_back();
            }
        }

        ListNode* newHead = list_.push_front({key, value});
        map_.insert(key, newHead);
    }
};

} // namespace dsa
