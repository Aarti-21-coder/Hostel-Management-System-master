#pragma once

#include <string>

namespace hostel {

enum class ComplaintStatus {
    OPEN,
    IN_PROGRESS,
    RESOLVED
};

inline std::string complaintStatusToString(ComplaintStatus s) {
    switch (s) {
        case ComplaintStatus::OPEN:        return "OPEN";
        case ComplaintStatus::IN_PROGRESS: return "IN_PROGRESS";
        case ComplaintStatus::RESOLVED:    return "RESOLVED";
        default:                           return "UNKNOWN";
    }
}

struct Complaint {
    std::string id;
    std::string studentId;
    std::string category;     // "Electrical", "Plumbing", "Cleanliness", "WiFi", "Mess"
    std::string description;
    int priority = 3;         // 1=Urgent, 2=High, 3=Medium, 4=Low
    int slaHours = 24;        // SLA deadline in hours
    int elapsedHours = 0;
    ComplaintStatus status = ComplaintStatus::OPEN;
    std::string createdAt;
    std::string resolutionNotes;

    Complaint() = default;

    Complaint(std::string cId, std::string sId, std::string cat,
              std::string desc, int prio, int sla = 24, std::string created = "")
        : id(std::move(cId)), studentId(std::move(sId)), category(std::move(cat)),
          description(std::move(desc)), priority(prio), slaHours(sla),
          createdAt(std::move(created)) {}

    // Min-Heap comparator: priority 1 (Urgent) is smaller than priority 4 (Low),
    // and smaller remaining SLA comes first
    bool operator>(const Complaint& other) const {
        if (priority != other.priority) {
            return priority > other.priority; // 1 has higher priority than 2
        }
        return (slaHours - elapsedHours) > (other.slaHours - other.elapsedHours);
    }

    bool operator<(const Complaint& other) const {
        if (priority != other.priority) {
            return priority < other.priority;
        }
        return (slaHours - elapsedHours) < (other.slaHours - other.elapsedHours);
    }

    bool operator==(const Complaint& other) const {
        return id == other.id;
    }
};

} // namespace hostel
