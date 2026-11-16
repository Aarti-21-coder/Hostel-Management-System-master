#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace dsa {

/**
 * @brief Doubly Linked List Data Structure.
 * 
 * Used for the Allocated Students DB and LRU Cache node management.
 * 
 * Time Complexities:
 *  - Push Front / Push Back: O(1)
 *  - Pop Front / Pop Back: O(1)
 *  - Remove given Node pointer: O(1)
 *  - Search / Traverse: O(N)
 *  - Empty / Size: O(1)
 * 
 * Space Complexity: O(N) for N nodes.
 */
template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        explicit Node(const T& val, Node* p = nullptr, Node* n = nullptr)
            : data(val), prev(p), next(n) {}
        explicit Node(T&& val, Node* p = nullptr, Node* n = nullptr)
            : data(std::move(val)), prev(p), next(n) {}
    };

private:
    Node* head_;
    Node* tail_;
    size_t size_;

public:
    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~DoublyLinkedList() {
        clear();
    }

    // Copy Constructor - O(N)
    DoublyLinkedList(const DoublyLinkedList& other) : head_(nullptr), tail_(nullptr), size_(0) {
        Node* curr = other.head_;
        while (curr != nullptr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }

    // Copy Assignment - O(N)
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            clear();
            Node* curr = other.head_;
            while (curr != nullptr) {
                push_back(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }

    // Move Constructor - O(1)
    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    // Move Assignment - O(1)
    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;

            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    Node* head() noexcept { return head_; }
    const Node* head() const noexcept { return head_; }
    Node* tail() noexcept { return tail_; }
    const Node* tail() const noexcept { return tail_; }

    void clear() noexcept {
        Node* curr = head_;
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

    // Insert at front - O(1)
    Node* push_front(const T& value) {
        Node* newNode = new Node(value, nullptr, head_);
        if (head_ != nullptr) {
            head_->prev = newNode;
        } else {
            tail_ = newNode;
        }
        head_ = newNode;
        ++size_;
        return newNode;
    }

    Node* push_front(T&& value) {
        Node* newNode = new Node(std::move(value), nullptr, head_);
        if (head_ != nullptr) {
            head_->prev = newNode;
        } else {
            tail_ = newNode;
        }
        head_ = newNode;
        ++size_;
        return newNode;
    }

    // Insert at back - O(1)
    Node* push_back(const T& value) {
        Node* newNode = new Node(value, tail_, nullptr);
        if (tail_ != nullptr) {
            tail_->next = newNode;
        } else {
            head_ = newNode;
        }
        tail_ = newNode;
        ++size_;
        return newNode;
    }

    Node* push_back(T&& value) {
        Node* newNode = new Node(std::move(value), tail_, nullptr);
        if (tail_ != nullptr) {
            tail_->next = newNode;
        } else {
            head_ = newNode;
        }
        tail_ = newNode;
        ++size_;
        return newNode;
    }

    // Pop from front - O(1)
    void pop_front() {
        if (empty()) {
            throw std::underflow_error("DoublyLinkedList::pop_front called on empty list");
        }
        Node* toDelete = head_;
        head_ = head_->next;
        if (head_ != nullptr) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }
        delete toDelete;
        --size_;
    }

    // Pop from back - O(1)
    void pop_back() {
        if (empty()) {
            throw std::underflow_error("DoublyLinkedList::pop_back called on empty list");
        }
        Node* toDelete = tail_;
        tail_ = tail_->prev;
        if (tail_ != nullptr) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }
        delete toDelete;
        --size_;
    }

    // Remove specific node by pointer - O(1)
    void removeNode(Node* node) {
        if (node == nullptr) return;

        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }

        delete node;
        --size_;
    }

    // Remove first node satisfying predicate - O(N)
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        Node* curr = head_;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                removeNode(curr);
                return true;
            }
            curr = curr->next;
        }
        return false;
    }

    // Search for first node satisfying predicate - O(N)
    template <typename Predicate>
    Node* findIf(Predicate pred) {
        Node* curr = head_;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    template <typename Predicate>
    const Node* findIf(Predicate pred) const {
        const Node* curr = head_;
        while (curr != nullptr) {
            if (pred(curr->data)) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    // Traversal callback - O(N)
    template <typename Func>
    void forEach(Func func) const {
        const Node* curr = head_;
        while (curr != nullptr) {
            func(curr->data);
            curr = curr->next;
        }
    }
};

} // namespace dsa
