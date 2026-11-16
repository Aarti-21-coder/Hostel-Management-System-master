#pragma once

#include <cstddef>
#include <utility>
#include <functional>

namespace dsa {

/**
 * @brief Sorting Metrics for DSA Lab & Benchmarks.
 */
struct SortMetrics {
    size_t comparisons = 0;
    size_t swaps = 0;
};

/**
 * @brief Custom QuickSort Implementation.
 * 
 * Uses median-of-three pivot selection to prevent worst-case O(N^2) on sorted data.
 * 
 * Time Complexity:
 *  - Average: O(N log N)
 *  - Best: O(N log N)
 *  - Worst: O(N^2) (mitigated by median-of-three)
 * Space Complexity: O(log N) recursion call stack.
 */
template <typename T, typename Compare = std::less<T>>
class QuickSort {
private:
    static size_t medianOfThree(T* arr, size_t low, size_t high, Compare comp, SortMetrics* metrics) {
        size_t mid = low + (high - low) / 2;
        if (metrics) metrics->comparisons += 3;

        if (comp(arr[mid], arr[low])) std::swap(arr[low], arr[mid]);
        if (comp(arr[high], arr[low])) std::swap(arr[low], arr[high]);
        if (comp(arr[high], arr[mid])) std::swap(arr[mid], arr[high]);

        return mid;
    }

    static size_t partition(T* arr, size_t low, size_t high, Compare comp, SortMetrics* metrics) {
        size_t pivotIdx = medianOfThree(arr, low, high, comp, metrics);
        std::swap(arr[pivotIdx], arr[high]);
        if (metrics) ++metrics->swaps;

        T pivot = arr[high];
        size_t i = low;

        for (size_t j = low; j < high; ++j) {
            if (metrics) ++metrics->comparisons;
            if (comp(arr[j], pivot)) {
                std::swap(arr[i], arr[j]);
                if (metrics) ++metrics->swaps;
                ++i;
            }
        }
        std::swap(arr[i], arr[high]);
        if (metrics) ++metrics->swaps;
        return i;
    }

    static void sortInternal(T* arr, size_t low, size_t high, Compare comp, SortMetrics* metrics) {
        if (low < high) {
            size_t p = partition(arr, low, high, comp, metrics);
            if (p > 0) {
                sortInternal(arr, low, p - 1, comp, metrics);
            }
            sortInternal(arr, p + 1, high, comp, metrics);
        }
    }

public:
    static void sort(T* arr, size_t n, Compare comp = Compare(), SortMetrics* metrics = nullptr) {
        if (n <= 1 || arr == nullptr) return;
        sortInternal(arr, 0, n - 1, comp, metrics);
    }
};

/**
 * @brief Custom MergeSort Implementation.
 * 
 * Stable divide-and-conquer sorting algorithm.
 * 
 * Time Complexity: Guaranteed O(N log N) in all cases (Best, Average, Worst).
 * Space Complexity: O(N) auxiliary buffer.
 */
template <typename T, typename Compare = std::less<T>>
class MergeSort {
private:
    static void merge(T* arr, T* temp, size_t left, size_t mid, size_t right, Compare comp, SortMetrics* metrics) {
        size_t i = left;
        size_t j = mid + 1;
        size_t k = left;

        while (i <= mid && j <= right) {
            if (metrics) ++metrics->comparisons;
            if (comp(arr[j], arr[i])) { // arr[j] < arr[i]
                temp[k++] = std::move(arr[j++]);
            } else {
                temp[k++] = std::move(arr[i++]);
            }
            if (metrics) ++metrics->swaps;
        }

        while (i <= mid) {
            temp[k++] = std::move(arr[i++]);
            if (metrics) ++metrics->swaps;
        }

        while (j <= right) {
            temp[k++] = std::move(arr[j++]);
            if (metrics) ++metrics->swaps;
        }

        for (size_t p = left; p <= right; ++p) {
            arr[p] = std::move(temp[p]);
        }
    }

    static void sortInternal(T* arr, T* temp, size_t left, size_t right, Compare comp, SortMetrics* metrics) {
        if (left < right) {
            size_t mid = left + (right - left) / 2;
            sortInternal(arr, temp, left, mid, comp, metrics);
            sortInternal(arr, temp, mid + 1, right, comp, metrics);
            merge(arr, temp, left, mid, right, comp, metrics);
        }
    }

public:
    static void sort(T* arr, size_t n, Compare comp = Compare(), SortMetrics* metrics = nullptr) {
        if (n <= 1 || arr == nullptr) return;
        T* temp = new T[n];
        sortInternal(arr, temp, 0, n - 1, comp, metrics);
        delete[] temp;
    }
};

/**
 * @brief Custom HeapSort Implementation.
 * 
 * In-place comparison-based sorting algorithm using a binary heap structure.
 * 
 * Time Complexity: O(N log N) Best, Average, Worst.
 * Space Complexity: O(1) auxiliary space.
 */
template <typename T, typename Compare = std::less<T>>
class HeapSort {
private:
    static void heapify(T* arr, size_t n, size_t i, Compare comp, SortMetrics* metrics) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left < n) {
            if (metrics) ++metrics->comparisons;
            if (comp(arr[largest], arr[left])) {
                largest = left;
            }
        }

        if (right < n) {
            if (metrics) ++metrics->comparisons;
            if (comp(arr[largest], arr[right])) {
                largest = right;
            }
        }

        if (largest != i) {
            std::swap(arr[i], arr[largest]);
            if (metrics) ++metrics->swaps;
            heapify(arr, n, largest, comp, metrics);
        }
    }

public:
    static void sort(T* arr, size_t n, Compare comp = Compare(), SortMetrics* metrics = nullptr) {
        if (n <= 1 || arr == nullptr) return;

        // Build max heap - O(N)
        for (int i = static_cast<int>(n / 2) - 1; i >= 0; --i) {
            heapify(arr, n, static_cast<size_t>(i), comp, metrics);
        }

        // Extract elements from heap one by one - O(N log N)
        for (size_t i = n - 1; i > 0; --i) {
            std::swap(arr[0], arr[i]);
            if (metrics) ++metrics->swaps;
            heapify(arr, i, 0, comp, metrics);
        }
    }
};

/**
 * @brief Custom Binary Search on sorted random-access data.
 * 
 * Time Complexity: O(log N).
 * Space Complexity: O(1).
 */
template <typename T, typename Compare = std::less<T>>
int binarySearch(const T* arr, size_t n, const T& target, Compare comp = Compare()) {
    int left = 0;
    int right = static_cast<int>(n) - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (!comp(arr[mid], target) && !comp(target, arr[mid])) {
            return mid; // match found
        }
        if (comp(arr[mid], target)) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1; // not found
}

} // namespace dsa
