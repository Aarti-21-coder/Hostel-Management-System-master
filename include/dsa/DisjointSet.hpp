#pragma once

#include <cstddef>
#include <numeric>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Disjoint Set Union (DSU / Union-Find).
 * 
 * Used for Smart Roommate Matching and student affinity clustering.
 * Implements Path Compression and Union by Rank.
 * 
 * Time Complexities:
 *  - Find: Amortized O(alpha(N)) ~ O(1)
 *  - Union: Amortized O(alpha(N)) ~ O(1)
 *  - Connected Check: Amortized O(alpha(N)) ~ O(1)
 * 
 * Space Complexity: O(N).
 */
class DisjointSet {
private:
    Vector<int> parent_;
    Vector<int> rank_;
    size_t numSets_;

public:
    explicit DisjointSet(size_t n = 0) : numSets_(n) {
        parent_.reserve(n);
        rank_.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            parent_.push_back(static_cast<int>(i));
            rank_.push_back(0);
        }
    }

    void addElement() {
        int id = static_cast<int>(parent_.size());
        parent_.push_back(id);
        rank_.push_back(0);
        ++numSets_;
    }

    // Find with Path Compression - O(alpha(N))
    int find(int i) {
        if (i < 0 || static_cast<size_t>(i) >= parent_.size()) return -1;
        if (parent_[i] == i) {
            return i;
        }
        return parent_[i] = find(parent_[i]);
    }

    // Union by Rank - O(alpha(N))
    bool unite(int i, int j) {
        int rootI = find(i);
        int rootJ = find(j);

        if (rootI == -1 || rootJ == -1 || rootI == rootJ) {
            return false; // Already in same set or invalid
        }

        if (rank_[rootI] < rank_[rootJ]) {
            parent_[rootI] = rootJ;
        } else if (rank_[rootI] > rank_[rootJ]) {
            parent_[rootJ] = rootI;
        } else {
            parent_[rootJ] = rootI;
            ++rank_[rootI];
        }

        --numSets_;
        return true;
    }

    [[nodiscard]] bool connected(int i, int j) {
        return find(i) == find(j) && find(i) != -1;
    }

    [[nodiscard]] size_t countSets() const noexcept {
        return numSets_;
    }

    [[nodiscard]] size_t size() const noexcept {
        return parent_.size();
    }
};

} // namespace dsa
