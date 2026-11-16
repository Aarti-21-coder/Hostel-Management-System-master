#pragma once

#include <algorithm>
#include <cstddef>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Interval representation [low, high].
 */
template <typename T>
struct Interval {
    T low;
    T high;
    std::string tag; // e.g. Student ID or Booking Ref

    Interval() : low(T{0}), high(T{0}) {}
    Interval(T l, T h, std::string t = "") : low(l), high(h), tag(std::move(t)) {}

    [[nodiscard]] bool overlaps(const Interval& other) const noexcept {
        return (low <= other.high && other.low <= high);
    }
};

/**
 * @brief Augmented Interval Tree.
 * 
 * Used for detecting booking date conflicts and overlapping student leave requests.
 * 
 * Time Complexities:
 *  - Insert: O(log N)
 *  - Find Any Overlap: O(log N)
 *  - Find All Overlaps: O(log N + k), where k is the number of overlapping intervals
 * 
 * Space Complexity: O(N).
 */
template <typename T>
class IntervalTree {
public:
    struct Node {
        Interval<T> interval;
        T maxHigh;
        Node* left;
        Node* right;

        explicit Node(const Interval<T>& i)
            : interval(i), maxHigh(i.high), left(nullptr), right(nullptr) {}
    };

private:
    Node* root_;
    size_t size_;

    Node* insertInternal(Node* node, const Interval<T>& interval) {
        if (!node) {
            return new Node(interval);
        }

        if (interval.low < node->interval.low) {
            node->left = insertInternal(node->left, interval);
        } else {
            node->right = insertInternal(node->right, interval);
        }

        if (node->maxHigh < interval.high) {
            node->maxHigh = interval.high;
        }

        return node;
    }

    void findOverlapsInternal(Node* node, const Interval<T>& target, Vector<Interval<T>>& result) const {
        if (!node) return;

        if (node->interval.overlaps(target)) {
            result.push_back(node->interval);
        }

        // If left child has a maxHigh >= target.low, an overlap could be in the left subtree
        if (node->left && node->left->maxHigh >= target.low) {
            findOverlapsInternal(node->left, target, result);
        }

        // An overlap could also be in the right subtree
        findOverlapsInternal(node->right, target, result);
    }

    void freeAll(Node* n) {
        if (n) {
            freeAll(n->left);
            freeAll(n->right);
            delete n;
        }
    }

public:
    IntervalTree() : root_(nullptr), size_(0) {}

    ~IntervalTree() {
        freeAll(root_);
    }

    IntervalTree(const IntervalTree&) = delete;
    IntervalTree& operator=(const IntervalTree&) = delete;

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void clear() {
        freeAll(root_);
        root_ = nullptr;
        size_ = 0;
    }

    // Insert interval - O(log N)
    void insert(const Interval<T>& interval) {
        root_ = insertInternal(root_, interval);
        ++size_;
    }

    void insert(T low, T high, const std::string& tag = "") {
        insert(Interval<T>(low, high, tag));
    }

    // Check if an interval overlaps with any existing interval - O(log N)
    [[nodiscard]] bool hasOverlap(const Interval<T>& target) const {
        Node* curr = root_;
        while (curr) {
            if (curr->interval.overlaps(target)) {
                return true;
            }
            if (curr->left && curr->left->maxHigh >= target.low) {
                curr = curr->left;
            } else {
                curr = curr->right;
            }
        }
        return false;
    }

    // Find all overlapping intervals - O(log N + k)
    Vector<Interval<T>> findAllOverlaps(const Interval<T>& target) const {
        Vector<Interval<T>> result;
        findOverlapsInternal(root_, target, result);
        return result;
    }
};

} // namespace dsa
