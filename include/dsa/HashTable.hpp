#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <stdexcept>
#include <functional>

namespace dsa {

// Default hash functors
template <typename K>
struct Hash {
    size_t operator()(const K& key) const {
        return std::hash<K>{}(key);
    }
};

// Specialized FNV-1a hash for std::string
template <>
struct Hash<std::string> {
    size_t operator()(const std::string& key) const {
        size_t hash = 14695981039346656037ULL; // FNV offset basis
        for (char c : key) {
            hash ^= static_cast<size_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ULL; // FNV prime
        }
        return hash;
    }
};

/**
 * @brief Hash Table using Separate Chaining with Automatic Dynamic Rehashing.
 * 
 * Time Complexities:
 *  - Insert: Average O(1), Worst O(N) on full bucket collision / rehash
 *  - Search / Find: Average O(1), Worst O(N)
 *  - Erase: Average O(1), Worst O(N)
 * 
 * Space Complexity: O(N + M), where N is number of elements, M is bucket count.
 */
template <typename K, typename V, typename HashFunc = Hash<K>>
class HashTable {
public:
    struct Entry {
        K key;
        V value;
        Entry* next;

        Entry(const K& k, const V& v, Entry* n = nullptr) 
            : key(k), value(v), next(n) {}
        Entry(K&& k, V&& v, Entry* n = nullptr) 
            : key(std::move(k)), value(std::move(v)), next(n) {}
    };

private:
    Entry** buckets_;
    size_t bucketCount_;
    size_t size_;
    float maxLoadFactor_;
    HashFunc hasher_;

    // Prime numbers for bucket sizes to minimize modulo collisions
    static constexpr size_t PRIMES[] = {
        11, 23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
        49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469
    };
    static constexpr size_t NUM_PRIMES = sizeof(PRIMES) / sizeof(PRIMES[0]);

    size_t getNextPrime(size_t current) const {
        for (size_t i = 0; i < NUM_PRIMES; ++i) {
            if (PRIMES[i] > current) {
                return PRIMES[i];
            }
        }
        return current * 2 + 1;
    }

    size_t bucketIndex(const K& key, size_t bucketCount) const {
        return hasher_(key) % bucketCount;
    }

    void rehash(size_t newBucketCount) {
        Entry** newBuckets = new Entry*[newBucketCount]();

        for (size_t i = 0; i < bucketCount_; ++i) {
            Entry* curr = buckets_[i];
            while (curr != nullptr) {
                Entry* nextEntry = curr->next;
                size_t newIdx = hasher_(curr->key) % newBucketCount;
                curr->next = newBuckets[newIdx];
                newBuckets[newIdx] = curr;
                curr = nextEntry;
            }
        }

        delete[] buckets_;
        buckets_ = newBuckets;
        bucketCount_ = newBucketCount;
    }

    void freeAll() {
        if (buckets_ != nullptr) {
            for (size_t i = 0; i < bucketCount_; ++i) {
                Entry* curr = buckets_[i];
                while (curr != nullptr) {
                    Entry* next = curr->next;
                    delete curr;
                    curr = next;
                }
            }
            delete[] buckets_;
            buckets_ = nullptr;
        }
        size_ = 0;
    }

public:
    explicit HashTable(size_t initialBuckets = 17, float maxLoadFactor = 0.75f)
        : buckets_(nullptr), bucketCount_(initialBuckets), size_(0), 
          maxLoadFactor_(maxLoadFactor), hasher_() {
        if (bucketCount_ == 0) bucketCount_ = 11;
        buckets_ = new Entry*[bucketCount_]();
    }

    ~HashTable() {
        freeAll();
    }

    // Copy Constructor - O(N)
    HashTable(const HashTable& other)
        : buckets_(nullptr), bucketCount_(other.bucketCount_), size_(0),
          maxLoadFactor_(other.maxLoadFactor_), hasher_(other.hasher_) {
        buckets_ = new Entry*[bucketCount_]();
        for (size_t i = 0; i < other.bucketCount_; ++i) {
            Entry* curr = other.buckets_[i];
            while (curr != nullptr) {
                insert(curr->key, curr->value);
                curr = curr->next;
            }
        }
    }

    // Copy Assignment - O(N)
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            freeAll();
            bucketCount_ = other.bucketCount_;
            maxLoadFactor_ = other.maxLoadFactor_;
            hasher_ = other.hasher_;
            buckets_ = new Entry*[bucketCount_]();
            for (size_t i = 0; i < other.bucketCount_; ++i) {
                Entry* curr = other.buckets_[i];
                while (curr != nullptr) {
                    insert(curr->key, curr->value);
                    curr = curr->next;
                }
            }
        }
        return *this;
    }

    // Move Constructor - O(1)
    HashTable(HashTable&& other) noexcept
        : buckets_(other.buckets_), bucketCount_(other.bucketCount_),
          size_(other.size_), maxLoadFactor_(other.maxLoadFactor_), hasher_(other.hasher_) {
        other.buckets_ = nullptr;
        other.bucketCount_ = 0;
        other.size_ = 0;
    }

    // Move Assignment - O(1)
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            freeAll();
            buckets_ = other.buckets_;
            bucketCount_ = other.bucketCount_;
            size_ = other.size_;
            maxLoadFactor_ = other.maxLoadFactor_;
            hasher_ = other.hasher_;

            other.buckets_ = nullptr;
            other.bucketCount_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] size_t bucket_count() const noexcept { return bucketCount_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] float load_factor() const noexcept {
        return bucketCount_ > 0 ? static_cast<float>(size_) / bucketCount_ : 0.0f;
    }

    void clear() {
        freeAll();
        bucketCount_ = 11;
        buckets_ = new Entry*[bucketCount_]();
    }

    // Insert or update value - Avg O(1)
    bool insert(const K& key, const V& value) {
        if (load_factor() >= maxLoadFactor_) {
            rehash(getNextPrime(bucketCount_));
        }

        size_t idx = bucketIndex(key, bucketCount_);
        Entry* curr = buckets_[idx];

        while (curr != nullptr) {
            if (curr->key == key) {
                curr->value = value;
                return false; // key already existed, updated
            }
            curr = curr->next;
        }

        buckets_[idx] = new Entry(key, value, buckets_[idx]);
        ++size_;
        return true; // newly inserted
    }

    bool insert(K&& key, V&& value) {
        if (load_factor() >= maxLoadFactor_) {
            rehash(getNextPrime(bucketCount_));
        }

        size_t idx = bucketIndex(key, bucketCount_);
        Entry* curr = buckets_[idx];

        while (curr != nullptr) {
            if (curr->key == key) {
                curr->value = std::move(value);
                return false;
            }
            curr = curr->next;
        }

        buckets_[idx] = new Entry(std::move(key), std::move(value), buckets_[idx]);
        ++size_;
        return true;
    }

    // Lookup value pointer - Avg O(1)
    V* find(const K& key) {
        if (bucketCount_ == 0) return nullptr;
        size_t idx = bucketIndex(key, bucketCount_);
        Entry* curr = buckets_[idx];
        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    const V* find(const K& key) const {
        if (bucketCount_ == 0) return nullptr;
        size_t idx = bucketIndex(key, bucketCount_);
        Entry* curr = buckets_[idx];
        while (curr != nullptr) {
            if (curr->key == key) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    [[nodiscard]] bool contains(const K& key) const {
        return find(key) != nullptr;
    }

    // Subscript operator with default creation - Avg O(1)
    V& operator[](const K& key) {
        V* val = find(key);
        if (val != nullptr) {
            return *val;
        }
        insert(key, V{});
        return *find(key);
    }

    // Erase by key - Avg O(1)
    bool erase(const K& key) {
        if (bucketCount_ == 0) return false;
        size_t idx = bucketIndex(key, bucketCount_);
        Entry* curr = buckets_[idx];
        Entry* prev = nullptr;

        while (curr != nullptr) {
            if (curr->key == key) {
                if (prev == nullptr) {
                    buckets_[idx] = curr->next;
                } else {
                    prev->next = curr->next;
                }
                delete curr;
                --size_;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    // Apply callback function to each key-value pair
    template <typename Func>
    void forEach(Func func) const {
        for (size_t i = 0; i < bucketCount_; ++i) {
            Entry* curr = buckets_[i];
            while (curr != nullptr) {
                func(curr->key, curr->value);
                curr = curr->next;
            }
        }
    }
};

} // namespace dsa
