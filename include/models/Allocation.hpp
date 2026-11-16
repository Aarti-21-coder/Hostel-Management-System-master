#pragma once

#include <string>

namespace hostel {

/**
 * @brief Record in the Allocated Students DB (Doubly Linked List).
 */
struct AllocationRecord {
    std::string studentId;
    std::string roomId;
    std::string studentName;
    std::string allocatedDate;
    int bedIndex = 1;

    AllocationRecord() = default;

    AllocationRecord(const std::string& sId, const std::string& rId,
                     const std::string& sName = "", const std::string& aDate = "",
                     int bed = 1)
        : studentId(sId), roomId(rId), studentName(sName),
          allocatedDate(aDate), bedIndex(bed) {}

    bool operator==(const AllocationRecord& other) const {
        return studentId == other.studentId && roomId == other.roomId;
    }
};

} // namespace hostel
