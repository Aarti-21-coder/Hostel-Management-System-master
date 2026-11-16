#pragma once

#include <string>
#include <algorithm>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Prefix Tree (Trie) for Instant Autocomplete & Typo-Tolerant Search.
 * 
 * Time Complexities:
 *  - Insert: O(L) where L is string length
 *  - Search: O(L)
 *  - Autocomplete: O(P + K) where P is prefix length, K is number of matched characters
 *  - Typo Tolerant Search: O(Sigma^D * L) where D is maximum Levenshtein distance
 * 
 * Space Complexity: O(Sigma * L * N).
 */
class Trie {
public:
    static constexpr size_t ALPHABET_SIZE = 128; // Standard ASCII

    struct Node {
        Node* children[ALPHABET_SIZE];
        bool isEndOfWord;
        std::string payload; // Associated Student ID or entity key

        Node() : isEndOfWord(false), payload("") {
            for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
                children[i] = nullptr;
            }
        }
    };

    struct MatchResult {
        std::string word;
        std::string payload;
        int distance; // 0 for exact/autocomplete, >0 for typos
    };

private:
    Node* root_;
    size_t size_;

    static std::string toLower(const std::string& str) {
        std::string res = str;
        for (char& c : res) {
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
        }
        return res;
    }

    void collectAllWords(Node* node, const std::string& currentWord,
                         Vector<MatchResult>& results, size_t limit) const {
        if (!node || results.size() >= limit) return;

        if (node->isEndOfWord) {
            results.push_back({currentWord, node->payload, 0});
            if (results.size() >= limit) return;
        }

        for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
            if (node->children[i]) {
                collectAllWords(node->children[i], currentWord + static_cast<char>(i), results, limit);
            }
        }
    }

    void searchFuzzyInternal(Node* node, char ch, const std::string& currentWord,
                             const std::string& query, const Vector<int>& prevRow,
                             int maxDist, Vector<MatchResult>& results, size_t limit) const {
        size_t cols = query.size() + 1;
        Vector<int> currRow(cols, 0);
        currRow[0] = prevRow[0] + 1;

        for (size_t col = 1; col < cols; ++col) {
            int insertCost = currRow[col - 1] + 1;
            int deleteCost = prevRow[col] + 1;
            int replaceCost = prevRow[col - 1] + (query[col - 1] == ch ? 0 : 1);
            currRow[col] = std::min({insertCost, deleteCost, replaceCost});
        }

        if (currRow[cols - 1] <= maxDist && node->isEndOfWord) {
            results.push_back({currentWord, node->payload, currRow[cols - 1]});
        }

        int minInRow = currRow[0];
        for (size_t col = 1; col < cols; ++col) {
            if (currRow[col] < minInRow) minInRow = currRow[col];
        }

        if (minInRow <= maxDist && results.size() < limit) {
            for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
                if (node->children[i]) {
                    searchFuzzyInternal(node->children[i], static_cast<char>(i),
                                        currentWord + static_cast<char>(i), query,
                                        currRow, maxDist, results, limit);
                }
            }
        }
    }

    void freeAll(Node* n) {
        if (n) {
            for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
                freeAll(n->children[i]);
            }
            delete n;
        }
    }

public:
    Trie() : root_(new Node()), size_(0) {}

    ~Trie() {
        freeAll(root_);
    }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    [[nodiscard]] size_t size() const noexcept { return size_; }

    // Insert word and payload - O(L)
    void insert(const std::string& word, const std::string& payload = "") {
        if (word.empty()) return;
        std::string lower = toLower(word);
        Node* curr = root_;
        for (char c : lower) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc >= ALPHABET_SIZE) continue;
            if (!curr->children[uc]) {
                curr->children[uc] = new Node();
            }
            curr = curr->children[uc];
        }
        if (!curr->isEndOfWord) {
            curr->isEndOfWord = true;
            ++size_;
        }
        curr->payload = payload;
    }

    // Exact search - O(L)
    [[nodiscard]] bool search(const std::string& word) const {
        std::string lower = toLower(word);
        Node* curr = root_;
        for (char c : lower) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc >= ALPHABET_SIZE || !curr->children[uc]) {
                return false;
            }
            curr = curr->children[uc];
        }
        return curr && curr->isEndOfWord;
    }

    // Autocomplete matching prefix - O(P + K)
    Vector<MatchResult> autocomplete(const std::string& prefix, size_t limit = 10) const {
        Vector<MatchResult> results;
        std::string lower = toLower(prefix);
        Node* curr = root_;
        for (char c : lower) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc >= ALPHABET_SIZE || !curr->children[uc]) {
                return results; // No words match prefix
            }
            curr = curr->children[uc];
        }

        collectAllWords(curr, lower, results, limit);
        return results;
    }

    // Typo-tolerant search using Levenshtein Automata dynamic programming - O(Sigma^D * L)
    Vector<MatchResult> searchWithTypoTolerance(const std::string& query, int maxDistance = 2, size_t limit = 10) const {
        Vector<MatchResult> results;
        std::string lower = toLower(query);
        size_t cols = lower.size() + 1;
        Vector<int> initialRow(cols, 0);
        for (size_t i = 0; i < cols; ++i) {
            initialRow[i] = static_cast<int>(i);
        }

        for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
            if (root_->children[i]) {
                std::string s(1, static_cast<char>(i));
                searchFuzzyInternal(root_->children[i], static_cast<char>(i), s, lower, initialRow, maxDistance, results, limit);
            }
        }
        return results;
    }
};

} // namespace dsa
