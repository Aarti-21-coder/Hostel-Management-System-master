#pragma once

#include <string>

namespace hostel {

enum class RoomStatus {
    AVAILABLE,
    FULL,
    MAINTENANCE
};

inline std::string roomStatusToString(RoomStatus s) {
    switch (s) {
        case RoomStatus::AVAILABLE:   return "AVAILABLE";
        case RoomStatus::FULL:        return "FULL";
        case RoomStatus::MAINTENANCE: return "MAINTENANCE";
        default:                      return "UNKNOWN";
    }
}

inline RoomStatus stringToRoomStatus(const std::string& str) {
    if (str == "AVAILABLE")   return RoomStatus::AVAILABLE;
    if (str == "FULL")        return RoomStatus::FULL;
    if (str == "MAINTENANCE") return RoomStatus::MAINTENANCE;
    return RoomStatus::AVAILABLE;
}

/**
 * @brief Room Entity in the Room/Allocation Information Store.
 */
struct Room {
    std::string id;             // e.g. "A-101"
    std::string block;          // "Block A", "Block B", etc.
    int floor = 1;              // 1 to 4
    int capacity = 2;           // 1, 2, or 3
    int occupiedCount = 0;      // 0 to capacity
    std::string roomType;       // "Single", "Double", "Triple"
    bool isAC = false;          // AC / Non-AC
    std::string allowedGender;  // "Male", "Female", "Any"
    RoomStatus status = RoomStatus::AVAILABLE;
    float monthlyFee = 5000.0f; // Standard fee

    Room() = default;

    Room(const std::string& rId, const std::string& rBlock, int rFloor,
         int rCap, const std::string& rType, bool rAC,
         const std::string& rGender = "Any", float fee = 5000.0f)
        : id(rId), block(rBlock), floor(rFloor), capacity(rCap),
          occupiedCount(0), roomType(rType), isAC(rAC),
          allowedGender(rGender), status(RoomStatus::AVAILABLE),
          monthlyFee(fee) {}

    [[nodiscard]] bool isAvailable() const noexcept {
        return (status == RoomStatus::AVAILABLE) && (occupiedCount < capacity);
    }

    [[nodiscard]] bool canAccommodate(const std::string& studentGender) const {
        if (!isAvailable()) return false;
        if (allowedGender != "Any" && !allowedGender.empty()) {
            if (allowedGender != studentGender) return false;
        }
        return true;
    }

    bool allocateBed() {
        if (!isAvailable()) return false;
        ++occupiedCount;
        if (occupiedCount >= capacity) {
            status = RoomStatus::FULL;
        }
        return true;
    }

    bool freeBed() {
        if (occupiedCount > 0) {
            --occupiedCount;
            if (status == RoomStatus::FULL) {
                status = RoomStatus::AVAILABLE;
            }
            return true;
        }
        return false;
    }
};

} // namespace hostel
