#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <initializer_list>

namespace dsa {

/**
 * @brief Dynamic Array (Vector) Data Structure.
 * 
 * Time Complexities:
 *  - Random Access: O(1)
 *  - Append (push_back): Amortized O(1), Worst O(N) during reallocation
 *  - Pop Back: O(1)
 *  - Arbitrary Erase: O(N) due to element shifting
 *  - Search: O(N)
 * 
 * Space Complexity: O(N), where capacity >= size.
 */
template <typename T>
class Vector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void reallocate(size_t newCapacity) {
        if (newCapacity < size_) {
            newCapacity = size_;
        }
        if (newCapacity == 0) {
            newCapacity = 1;
        }

        T* newData = new T[newCapacity];
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }

        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

public:
    // Iterators
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept { return data_; }
    const_iterator begin() const noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator end() const noexcept { return data_ + size_; }

    // Constructors & Destructor
    Vector() : data_(nullptr), size_(0), capacity_(0) {
        reserve(8);
    }

    explicit Vector(size_t initialCapacity) : data_(nullptr), size_(0), capacity_(0) {
        reserve(initialCapacity);
    }

    Vector(size_t count, const T& value) : data_(nullptr), size_(0), capacity_(0) {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    Vector(std::initializer_list<T> init) : data_(nullptr), size_(0), capacity_(0) {
        reserve(init.size());
        for (const auto& item : init) {
            push_back(item);
        }
    }

    ~Vector() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // Copy Constructor - O(N)
    Vector(const Vector& other) : data_(nullptr), size_(0), capacity_(0) {
        reserve(other.capacity_);
        size_ = other.size_;
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy Assignment - O(N)
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            delete[] data_;
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;

            reserve(other.capacity_);
            size_ = other.size_;
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }

    // Move Constructor - O(1)
    Vector(Vector&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment - O(1)
    Vector& operator=(Vector&& other) noexcept {
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

    // Element Access - O(1)
    T& operator[](size_t index) noexcept {
        return data_[index];
    }

    const T& operator[](size_t index) const noexcept {
        return data_[index];
    }

    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Vector::at index out of range");
        }
        return data_[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Vector::at index out of range");
        }
        return data_[index];
    }

    T& front() {
        if (size_ == 0) throw std::out_of_range("Vector::front called on empty vector");
        return data_[0];
    }

    const T& front() const {
        if (size_ == 0) throw std::out_of_range("Vector::front called on empty vector");
        return data_[0];
    }

    T& back() {
        if (size_ == 0) throw std::out_of_range("Vector::back called on empty vector");
        return data_[size_ - 1];
    }

    const T& back() const {
        if (size_ == 0) throw std::out_of_range("Vector::back called on empty vector");
        return data_[size_ - 1];
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    // Capacity Queries & Modifiers
    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void reserve(size_t newCapacity) {
        if (newCapacity > capacity_) {
            reallocate(newCapacity);
        }
    }

    void shrink_to_fit() {
        if (capacity_ > size_) {
            reallocate(size_);
        }
    }

    void clear() noexcept {
        size_ = 0;
    }

    // Modifiers
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            size_t newCap = (capacity_ == 0) ? 8 : capacity_ * 2;
            reallocate(newCap);
        }
        data_[size_++] = value;
    }

    void push_back(T&& value) {
        if (size_ >= capacity_) {
            size_t newCap = (capacity_ == 0) ? 8 : capacity_ * 2;
            reallocate(newCap);
        }
        data_[size_++] = std::move(value);
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
        }
    }

    // Erase element at index - O(N)
    void erase(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Vector::erase index out of range");
        }
        for (size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        --size_;
    }
};

} // namespace dsa
