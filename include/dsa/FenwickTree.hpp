#pragma once

#include <cstddef>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Fenwick Tree (Binary Indexed Tree).
 * 
 * Space-efficient data structure for fast prefix sums and point updates.
 * 
 * Time Complexities:
 *  - Point Update (add delta): O(log N)
 *  - Prefix Sum Query [1, idx]: O(log N)
 *  - Range Sum Query [L, R]: O(log N)
 * 
 * Space Complexity: O(N).
 */
template <typename T>
class FenwickTree {
private:
    Vector<T> tree_;
    size_t n_;

public:
    explicit FenwickTree(size_t size = 0) : n_(size) {
        tree_.reserve(n_ + 1);
        for (size_t i = 0; i <= n_; ++i) {
            tree_.push_back(T{0});
        }
    }

    [[nodiscard]] size_t size() const noexcept { return n_; }

    // Point Update: adds delta to element at 1-based index - O(log N)
    void update(size_t idx, const T& delta) {
        for (; idx <= n_; idx += (idx & -idx)) {
            tree_[idx] = tree_[idx] + delta;
        }
    }

    // Prefix sum from 1 to idx - O(log N)
    T query(size_t idx) const {
        T sum{0};
        for (; idx > 0; idx -= (idx & -idx)) {
            sum = sum + tree_[idx];
        }
        return sum;
    }

    // Range sum query [left, right] (1-based indices) - O(log N)
    T rangeQuery(size_t left, size_t right) const {
        if (left > right || left == 0 || right > n_) return T{0};
        return query(right) - query(left - 1);
    }
};

} // namespace dsa
