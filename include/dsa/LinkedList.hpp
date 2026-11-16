#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace dsa {

/**
 * @brief Singly Linked List Data Structure.
 * 
 * Used for sequential event logs, visitor records, and audit history.
 * 
 * Time Complexities:
 *  - Push Front: O(1)
 *  - Push Back: O(1) with tail pointer
 *  - Pop Front: O(1)
 *  - Traversal: O(N)
 * 
 * Space Complexity: O(N).
 */
template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* next;

        explicit Node(const T& val, Node* n = nullptr) : data(val), next(n) {}
        explicit Node(T&& val, Node* n = nullptr) : data(std::move(val)), next(n) {}
    };

private:
    Node* head_;
    Node* tail_;
    size_t size_;

public:
    LinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~LinkedList() {
        clear();
    }

    LinkedList(const LinkedList& other) : head_(nullptr), tail_(nullptr), size_(0) {
        Node* curr = other.head_;
        while (curr) {
            push_back(curr->data);
            curr = curr->next;
        }
    }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            Node* curr = other.head_;
            while (curr) {
                push_back(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }

    LinkedList(LinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    LinkedList& operator=(LinkedList&& other) noexcept {
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

    void clear() noexcept {
        Node* curr = head_;
        while (curr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
    }

    void push_front(const T& val) {
        Node* n = new Node(val, head_);
        head_ = n;
        if (!tail_) tail_ = n;
        ++size_;
    }

    void push_front(T&& val) {
        Node* n = new Node(std::move(val), head_);
        head_ = n;
        if (!tail_) tail_ = n;
        ++size_;
    }

    void push_back(const T& val) {
        Node* n = new Node(val, nullptr);
        if (tail_) {
            tail_->next = n;
            tail_ = n;
        } else {
            head_ = tail_ = n;
        }
        ++size_;
    }

    void push_back(T&& val) {
        Node* n = new Node(std::move(val), nullptr);
        if (tail_) {
            tail_->next = n;
            tail_ = n;
        } else {
            head_ = tail_ = n;
        }
        ++size_;
    }

    void pop_front() {
        if (empty()) throw std::underflow_error("LinkedList::pop_front on empty list");
        Node* toDelete = head_;
        head_ = head_->next;
        if (!head_) tail_ = nullptr;
        delete toDelete;
        --size_;
    }

    template <typename Func>
    void forEach(Func func) const {
        Node* curr = head_;
        while (curr) {
            func(curr->data);
            curr = curr->next;
        }
    }
};

} // namespace dsa
