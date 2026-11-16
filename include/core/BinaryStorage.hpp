#pragma once

#include <string>
#include <fstream>
#include "core/HostelSystem.hpp"

namespace hostel {

/**
 * @brief High-Speed Binary Serializer & Deserializer for Cold Restarts.
 * 
 * Capable of saving and restoring 100,000+ student and room states in milliseconds.
 */
class BinaryStorage {
private:
    static constexpr uint32_t MAGIC_HEADER = 0x4853544C; // "HSTL"
    static constexpr uint32_t STORAGE_VERSION = 1;

    static void writeString(std::ofstream& ofs, const std::string& str);
    static std::string readString(std::ifstream& ifs);

public:
    static bool saveToFile(const HostelSystem& system, const std::string& filepath);
    static bool loadFromFile(HostelSystem& system, const std::string& filepath);
};

} // namespace hostel
