#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace dsa {

/**
 * @brief Circular Queue (Ring Buffer) Data Structure.
 * 
 * Used for the Normal Waiting List (FIFO) and SOS Alert Queue.
 * 
 * Time Complexities:
 *  - Enqueue: O(1)
 *  - Dequeue: O(1)
 *  - Front / Peek: O(1)
 *  - Size / Empty / Full check: O(1)
 *  - Remove Item: O(N) linear scan and compaction
 * 
 * Space Complexity: O(Capacity).
 */
template <typename T>
class CircularQueue {
private:
    T* buffer_;
    size_t capacity_;
    size_t front_;
    size_t rear_;
    size_t count_;

public:
    explicit CircularQueue(size_t capacity = 1000)
        : buffer_(nullptr), capacity_(capacity), front_(0), rear_(0), count_(0) {
        if (capacity_ == 0) capacity_ = 1;
        buffer_ = new T[capacity_];
    }

    ~CircularQueue() {
        delete[] buffer_;
        buffer_ = nullptr;
        capacity_ = 0;
        count_ = 0;
    }

    // Copy Constructor - O(N)
    CircularQueue(const CircularQueue& other)
        : buffer_(nullptr), capacity_(other.capacity_), front_(0), rear_(0), count_(other.count_) {
        buffer_ = new T[capacity_];
        for (size_t i = 0; i < count_; ++i) {
            size_t srcIdx = (other.front_ + i) % other.capacity_;
            buffer_[i] = other.buffer_[srcIdx];
        }
        rear_ = count_ % capacity_;
    }

    // Copy Assignment - O(N)
    CircularQueue& operator=(const CircularQueue& other) {
        if (this != &other) {
            delete[] buffer_;
            capacity_ = other.capacity_;
            count_ = other.count_;
            front_ = 0;
            buffer_ = new T[capacity_];
            for (size_t i = 0; i < count_; ++i) {
                size_t srcIdx = (other.front_ + i) % other.capacity_;
                buffer_[i] = other.buffer_[srcIdx];
            }
            rear_ = count_ % capacity_;
        }
        return *this;
    }

    // Move Constructor - O(1)
    CircularQueue(CircularQueue&& other) noexcept
        : buffer_(other.buffer_), capacity_(other.capacity_),
          front_(other.front_), rear_(other.rear_), count_(other.count_) {
        other.buffer_ = nullptr;
        other.capacity_ = 0;
        other.front_ = 0;
        other.rear_ = 0;
        other.count_ = 0;
    }

    // Move Assignment - O(1)
    CircularQueue& operator=(CircularQueue&& other) noexcept {
        if (this != &other) {
            delete[] buffer_;
            buffer_ = other.buffer_;
            capacity_ = other.capacity_;
            front_ = other.front_;
            rear_ = other.rear_;
            count_ = other.count_;

            other.buffer_ = nullptr;
            other.capacity_ = 0;
            other.front_ = 0;
            other.rear_ = 0;
            other.count_ = 0;
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return count_; }
    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }
    [[nodiscard]] bool full() const noexcept { return count_ == capacity_; }

    void clear() noexcept {
        front_ = 0;
        rear_ = 0;
        count_ = 0;
    }

    // Enqueue an element - O(1)
    // Throws std::overflow_error if full
    void enqueue(const T& value) {
        if (full()) {
            throw std::overflow_error("CircularQueue::enqueue called on full queue");
        }
        buffer_[rear_] = value;
        rear_ = (rear_ + 1) % capacity_;
        ++count_;
    }

    void enqueue(T&& value) {
        if (full()) {
            throw std::overflow_error("CircularQueue::enqueue called on full queue");
        }
        buffer_[rear_] = std::move(value);
        rear_ = (rear_ + 1) % capacity_;
        ++count_;
    }

    // Non-throwing enqueue attempt - returns false if full
    bool tryEnqueue(const T& value) {
        if (full()) return false;
        buffer_[rear_] = value;
        rear_ = (rear_ + 1) % capacity_;
        ++count_;
        return true;
    }

    bool tryEnqueue(T&& value) {
        if (full()) return false;
        buffer_[rear_] = std::move(value);
        rear_ = (rear_ + 1) % capacity_;
        ++count_;
        return true;
    }

    // Dequeue front element - O(1)
    T dequeue() {
        if (empty()) {
            throw std::underflow_error("CircularQueue::dequeue called on empty queue");
        }
        T item = std::move(buffer_[front_]);
        front_ = (front_ + 1) % capacity_;
        --count_;
        return item;
    }

    // Inspect front element - O(1)
    const T& front() const {
        if (empty()) {
            throw std::underflow_error("CircularQueue::front called on empty queue");
        }
        return buffer_[front_];
    }

    T& front() {
        if (empty()) {
            throw std::underflow_error("CircularQueue::front called on empty queue");
        }
        return buffer_[front_];
    }

    // Remove element matching predicate (e.g. For cancellation) - O(N)
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        if (empty()) return false;

        size_t foundIdx = capacity_;
        for (size_t i = 0; i < count_; ++i) {
            size_t idx = (front_ + i) % capacity_;
            if (pred(buffer_[idx])) {
                foundIdx = i;
                break;
            }
        }

        if (foundIdx == capacity_) {
            return false;
        }

        // Shift subsequent elements left by 1 position
        for (size_t i = foundIdx; i + 1 < count_; ++i) {
            size_t curr = (front_ + i) % capacity_;
            size_t next = (front_ + i + 1) % capacity_;
            buffer_[curr] = std::move(buffer_[next]);
        }

        --count_;
        rear_ = (front_ + count_) % capacity_;
        return true;
    }

    bool remove(const T& value) {
        return removeIf([&value](const T& item) { return item == value; });
    }

    // Iterate over elements from front to rear
    template <typename Func>
    void forEach(Func func) const {
        for (size_t i = 0; i < count_; ++i) {
            size_t idx = (front_ + i) % capacity_;
            func(buffer_[idx]);
        }
    }
};

} // namespace dsa
