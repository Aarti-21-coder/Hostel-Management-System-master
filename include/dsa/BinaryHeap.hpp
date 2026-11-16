#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <functional>

namespace dsa {

/**
 * @brief Generic Binary Heap (Priority Queue) supporting Min-Heap and Max-Heap.
 * 
 * When Compare is std::less<T>, this implements a MAX-HEAP (root is maximum).
 * When Compare is std::greater<T>, this implements a MIN-HEAP (root is minimum).
 * 
 * Time Complexities:
 *  - Top: O(1)
 *  - Push (Insert): O(log N)
 *  - Pop (Extract Root): O(log N)
 *  - Build Heap: O(N)
 *  - Remove Item: O(N) search + O(log N) reheapify
 * 
 * Space Complexity: O(N).
 */
template <typename T, typename Compare = std::less<T>>
class BinaryHeap {
private:
    T* data_;
    size_t size_;
    size_t capacity_;
    Compare comp_;

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

    void siftUp(size_t idx) {
        while (idx > 0) {
            size_t parent = (idx - 1) / 2;
            // In Max-Heap (std::less), if parent < child, swap them
            if (comp_(data_[parent], data_[idx])) {
                std::swap(data_[parent], data_[idx]);
                idx = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t idx) {
        while (true) {
            size_t left = 2 * idx + 1;
            size_t right = 2 * idx + 2;
            size_t best = idx;

            if (left < size_ && comp_(data_[best], data_[left])) {
                best = left;
            }
            if (right < size_ && comp_(data_[best], data_[right])) {
                best = right;
            }

            if (best != idx) {
                std::swap(data_[idx], data_[best]);
                idx = best;
            } else {
                break;
            }
        }
    }

public:
    explicit BinaryHeap(size_t initialCapacity = 16, Compare comp = Compare())
        : data_(nullptr), size_(0), capacity_(0), comp_(comp) {
        resize(initialCapacity);
    }

    ~BinaryHeap() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // Copy Constructor - O(N)
    BinaryHeap(const BinaryHeap& other)
        : data_(nullptr), size_(other.size_), capacity_(other.capacity_), comp_(other.comp_) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy Assignment - O(N)
    BinaryHeap& operator=(const BinaryHeap& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            comp_ = other.comp_;
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }

    // Move Constructor - O(1)
    BinaryHeap(BinaryHeap&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_), comp_(other.comp_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment - O(1)
    BinaryHeap& operator=(BinaryHeap&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            comp_ = other.comp_;

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

    // Peek root element - O(1)
    const T& top() const {
        if (empty()) {
            throw std::underflow_error("BinaryHeap::top called on empty heap");
        }
        return data_[0];
    }

    T& top() {
        if (empty()) {
            throw std::underflow_error("BinaryHeap::top called on empty heap");
        }
        return data_[0];
    }

    // Insert element - O(log N)
    void push(const T& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_] = value;
        siftUp(size_);
        ++size_;
    }

    void push(T&& value) {
        if (size_ >= capacity_) {
            resize(capacity_ * 2);
        }
        data_[size_] = std::move(value);
        siftUp(size_);
        ++size_;
    }

    // Extract root element - O(log N)
    void pop() {
        if (empty()) {
            throw std::underflow_error("BinaryHeap::pop called on empty heap");
        }
        data_[0] = std::move(data_[size_ - 1]);
        --size_;
        if (size_ > 0) {
            siftDown(0);
        }
    }

    // Rebuild full heap in O(N) using Floyd's algorithm
    void rebuild() {
        if (size_ <= 1) return;
        for (int i = static_cast<int>(size_ / 2) - 1; i >= 0; --i) {
            siftDown(static_cast<size_t>(i));
        }
    }

    // Remove element matching predicate - O(N + log N)
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        for (size_t i = 0; i < size_; ++i) {
            if (pred(data_[i])) {
                data_[i] = std::move(data_[size_ - 1]);
                --size_;
                if (i < size_) {
                    siftDown(i);
                    siftUp(i);
                }
                return true;
            }
        }
        return false;
    }

    // Linear search and remove by value
    bool remove(const T& value) {
        return removeIf([&value](const T& item) { return item == value; });
    }

    // Read-only access to underlying array (for reports/snapshots)
    const T* rawData() const noexcept { return data_; }
};

// Aliases for clear semantic naming
template <typename T>
using MaxHeap = BinaryHeap<T, std::less<T>>;

template <typename T>
using MinHeap = BinaryHeap<T, std::greater<T>>;

} // namespace dsa
