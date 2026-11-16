#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace dsa {

/**
 * @brief LIFO Stack Data Structure.
 * 
 * Used for Undo and Redo transaction stacks.
 * 
 * Time Complexities:
 *  - Push: Amortized O(1)
 *  - Pop: O(1)
 *  - Top: O(1)
 *  - Size / Empty: O(1)
 * 
 * Space Complexity: O(N).
 */
template <typename T>
class Stack {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void resize(size_t newCap) {
        if (newCap < size_) newCap = size_;
        if (newCap == 0) newCap = 1;

        T* newData = new T[newCap];
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = newData;
        capacity_ = newCap;
    }

public:
    explicit Stack(size_t initialCapacity = 16)
        : data_(nullptr), size_(0), capacity_(0) {
        resize(initialCapacity);
    }

    ~Stack() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // Copy Constructor - O(N)
    Stack(const Stack& other)
        : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy Assignment - O(N)
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }

    // Move Constructor - O(1)
    Stack(Stack&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment - O(1)
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void clear() noexcept {
        size_ = 0;
    }

    // Push element onto top of stack - Amortized O(1)
    void push(const T& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void push(T&& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_++] = std::move(value);
    }

    // Pop element from top of stack - O(1)
    void pop() {
        if (empty()) {
            throw std::underflow_error("Stack::pop called on empty stack");
        }
        --size_;
    }

    // Access top element - O(1)
    T& top() {
        if (empty()) {
            throw std::underflow_error("Stack::top called on empty stack");
        }
        return data_[size_ - 1];
    }

    const T& top() const {
        if (empty()) {
            throw std::underflow_error("Stack::top called on empty stack");
        }
        return data_[size_ - 1];
    }
};

} // namespace dsa
