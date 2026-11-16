#pragma once

#include <cstddef>
#include <string>
#include "core/HostelSystem.hpp"

namespace hostel {

/**
 * @brief High-Performance Synthetic Dataset Generator.
 * 
 * Generates realistic student records (1K, 10K, 100K) and 2000+ rooms.
 */
class DataGenerator {
public:
    static size_t generateRooms(HostelSystem& system, size_t targetCount = 2000);
    static size_t generateStudents(HostelSystem& system, size_t count = 1000);
};

} // namespace hostel
