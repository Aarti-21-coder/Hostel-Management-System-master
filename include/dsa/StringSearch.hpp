#pragma once

#include <string>
#include <cstddef>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Exact String Pattern Matching Algorithms (KMP & Rabin-Karp).
 * 
 * Used for log text inspection, complaint keyword scanning, and student search.
 */
class StringSearch {
public:
    /**
     * @brief Knuth-Morris-Pratt (KMP) Algorithm.
     * 
     * Uses LPS (Longest Prefix Suffix) array to avoid backtracking.
     * 
     * Time Complexity:
     *  - Preprocessing: O(M)
     *  - Matching: O(N)
     *  - Overall: O(N + M)
     * Space Complexity: O(M) for LPS array.
     */
    static Vector<int> computeLPS(const std::string& pattern) {
        size_t m = pattern.size();
        Vector<int> lps(m, 0);
        int len = 0;
        size_t i = 1;

        while (i < m) {
            if (pattern[i] == pattern[len]) {
                ++len;
                lps[i] = len;
                ++i;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    ++i;
                }
            }
        }
        return lps;
    }

    static int kmpFind(const std::string& text, const std::string& pattern) {
        if (pattern.empty()) return 0;
        if (text.empty() || pattern.size() > text.size()) return -1;

        Vector<int> lps = computeLPS(pattern);
        size_t n = text.size();
        size_t m = pattern.size();
        size_t i = 0; // index in text
        size_t j = 0; // index in pattern

        while (i < n) {
            if (pattern[j] == text[i]) {
                ++i;
                ++j;
            }

            if (j == m) {
                return static_cast<int>(i - j); // Match found at index (i - j)
            } else if (i < n && pattern[j] != text[i]) {
                if (j != 0) {
                    j = static_cast<size_t>(lps[j - 1]);
                } else {
                    ++i;
                }
            }
        }
        return -1; // Not found
    }

    static Vector<int> kmpFindAll(const std::string& text, const std::string& pattern) {
        Vector<int> matches;
        if (pattern.empty() || text.empty() || pattern.size() > text.size()) return matches;

        Vector<int> lps = computeLPS(pattern);
        size_t n = text.size();
        size_t m = pattern.size();
        size_t i = 0;
        size_t j = 0;

        while (i < n) {
            if (pattern[j] == text[i]) {
                ++i;
                ++j;
            }

            if (j == m) {
                matches.push_back(static_cast<int>(i - j));
                j = static_cast<size_t>(lps[j - 1]);
            } else if (i < n && pattern[j] != text[i]) {
                if (j != 0) {
                    j = static_cast<size_t>(lps[j - 1]);
                } else {
                    ++i;
                }
            }
        }
        return matches;
    }

    /**
     * @brief Rabin-Karp Algorithm using Rolling Hash.
     * 
     * Time Complexity:
     *  - Average / Best: O(N + M)
     *  - Worst: O(N * M) on pathological hash collisions
     * Space Complexity: O(1).
     */
    static int rabinKarpFind(const std::string& text, const std::string& pattern) {
        if (pattern.empty()) return 0;
        if (text.empty() || pattern.size() > text.size()) return -1;

        const uint64_t BASE = 256;
        const uint64_t MOD = 1000000007ULL;

        size_t n = text.size();
        size_t m = pattern.size();

        uint64_t pHash = 0; // Pattern hash
        uint64_t tHash = 0; // Text sliding window hash
        uint64_t h = 1;     // pow(BASE, m-1) % MOD

        for (size_t i = 0; i < m - 1; ++i) {
            h = (h * BASE) % MOD;
        }

        // Calculate initial hashes for pattern and first window
        for (size_t i = 0; i < m; ++i) {
            pHash = (BASE * pHash + static_cast<unsigned char>(pattern[i])) % MOD;
            tHash = (BASE * tHash + static_cast<unsigned char>(text[i])) % MOD;
        }

        // Slide the window over the text
        for (size_t i = 0; i <= n - m; ++i) {
            if (pHash == tHash) {
                // Hash matches, verify characters to eliminate spurious collisions
                bool match = true;
                for (size_t j = 0; j < m; ++j) {
                    if (text[i + j] != pattern[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) return static_cast<int>(i);
            }

            if (i < n - m) {
                // Rolling hash: remove leading char and add trailing char
                int64_t leading = (static_cast<unsigned char>(text[i]) * h) % MOD;
                int64_t nextHash = static_cast<int64_t>(tHash) - leading;
                if (nextHash < 0) nextHash += MOD;
                nextHash = (nextHash * BASE + static_cast<unsigned char>(text[i + m])) % MOD;
                tHash = static_cast<uint64_t>(nextHash);
            }
        }

        return -1; // Not found
    }
};

} // namespace dsa
