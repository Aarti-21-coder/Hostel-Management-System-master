#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include "dsa/Vector.hpp"

namespace dsa {

/**
 * @brief Bloom Filter Data Structure.
 * 
 * Space-efficient probabilistic filter with zero false negatives.
 * Used for instant duplicate detection of student emails and phone numbers.
 * 
 * Time Complexities:
 *  - Insert: O(k) where k is the number of hash functions
 *  - Contains (Query): O(k)
 * 
 * Space Complexity: O(M bits).
 */
class BloomFilter {
private:
    Vector<uint8_t> bitArray_;
    size_t numBits_;
    size_t numHashes_;

    static uint64_t fnv1a(const std::string& str, uint64_t seed) {
        uint64_t hash = 14695981039346656037ULL ^ seed;
        for (char c : str) {
            hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    static uint64_t murmurMix(uint64_t h) {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }

    size_t getHash(const std::string& str, size_t i) const {
        uint64_t h1 = fnv1a(str, 0x12345678ULL);
        uint64_t h2 = murmurMix(fnv1a(str, 0x87654321ULL));
        // Double hashing: hash_i = h1 + i * h2
        return static_cast<size_t>((h1 + i * h2) % numBits_);
    }

public:
    explicit BloomFilter(size_t numBits = 100000, size_t numHashes = 5)
        : numBits_(numBits), numHashes_(numHashes) {
        if (numBits_ == 0) numBits_ = 1024;
        size_t numBytes = (numBits_ + 7) / 8;
        bitArray_.reserve(numBytes);
        for (size_t i = 0; i < numBytes; ++i) {
            bitArray_.push_back(0);
        }
    }

    void clear() {
        for (size_t i = 0; i < bitArray_.size(); ++i) {
            bitArray_[i] = 0;
        }
    }

    // Insert key into filter - O(k)
    void insert(const std::string& key) {
        for (size_t i = 0; i < numHashes_; ++i) {
            size_t bitIdx = getHash(key, i);
            size_t byteIdx = bitIdx / 8;
            uint8_t bitMask = static_cast<uint8_t>(1 << (bitIdx % 8));
            bitArray_[byteIdx] |= bitMask;
        }
    }

    // Check if key might be in set - O(k)
    // Returns false: definitely not in set (0% false negative)
    // Returns true: probably in set (low false positive rate)
    [[nodiscard]] bool contains(const std::string& key) const {
        for (size_t i = 0; i < numHashes_; ++i) {
            size_t bitIdx = getHash(key, i);
            size_t byteIdx = bitIdx / 8;
            uint8_t bitMask = static_cast<uint8_t>(1 << (bitIdx % 8));
            if ((bitArray_[byteIdx] & bitMask) == 0) {
                return false;
            }
        }
        return true;
    }
};

} // namespace dsa
