#pragma once

#include <cstddef>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Segment Tree for Dynamic Range Sum Queries and Point Updates.
 * 
 * Used for dynamic fee dues aggregation and range statistics.
 * 
 * Time Complexities:
 *  - Construction: O(N)
 *  - Range Query [L, R]: O(log N)
 *  - Point Update: O(log N)
 * 
 * Space Complexity: O(4N).
 */
template <typename T>
class SegmentTree {
private:
    Vector<T> tree_;
    size_t n_;

    void buildInternal(const T* arr, size_t node, size_t start, size_t end) {
        if (start == end) {
            tree_[node] = arr[start];
            return;
        }
        size_t mid = start + (end - start) / 2;
        buildInternal(arr, 2 * node, start, mid);
        buildInternal(arr, 2 * node + 1, mid + 1, end);
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
    }

    void updateInternal(size_t node, size_t start, size_t end, size_t idx, const T& val) {
        if (start == end) {
            tree_[node] = val;
            return;
        }
        size_t mid = start + (end - start) / 2;
        if (idx <= mid) {
            updateInternal(2 * node, start, mid, idx, val);
        } else {
            updateInternal(2 * node + 1, mid + 1, end, idx, val);
        }
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
    }

    T queryInternal(size_t node, size_t start, size_t end, size_t l, size_t r) const {
        if (r < start || end < l) {
            return T{0}; // Identity value for sum
        }
        if (l <= start && end <= r) {
            return tree_[node];
        }
        size_t mid = start + (end - start) / 2;
        T p1 = queryInternal(2 * node, start, mid, l, r);
        T p2 = queryInternal(2 * node + 1, mid + 1, end, l, r);
        return p1 + p2;
    }

public:
    SegmentTree() : n_(0) {}

    explicit SegmentTree(size_t size) : n_(size) {
        tree_.reserve(4 * n_ + 1);
        for (size_t i = 0; i <= 4 * n_; ++i) {
            tree_.push_back(T{0});
        }
    }

    SegmentTree(const T* arr, size_t size) : n_(size) {
        tree_.reserve(4 * n_ + 1);
        for (size_t i = 0; i <= 4 * n_; ++i) {
            tree_.push_back(T{0});
        }
        if (n_ > 0 && arr != nullptr) {
            buildInternal(arr, 1, 0, n_ - 1);
        }
    }

    [[nodiscard]] size_t size() const noexcept { return n_; }

    // Point Update - O(log N)
    void update(size_t index, const T& val) {
        if (index < n_) {
            updateInternal(1, 0, n_ - 1, index, val);
        }
    }

    // Range Sum Query [left, right] inclusive - O(log N)
    T query(size_t left, size_t right) const {
        if (n_ == 0 || left > right || right >= n_) return T{0};
        return queryInternal(1, 0, n_ - 1, left, right);
    }
};

} // namespace dsa
