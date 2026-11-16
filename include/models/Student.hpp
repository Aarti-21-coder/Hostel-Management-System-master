#pragma once

#include <string>
#include <iostream>

namespace hostel {

enum class StudentStatus {
    REGISTERED,
    WAITING,
    ALLOCATED,
    VACATED,
    CANCELLED
};

inline std::string statusToString(StudentStatus status) {
    switch (status) {
        case StudentStatus::REGISTERED: return "REGISTERED";
        case StudentStatus::WAITING:    return "WAITING";
        case StudentStatus::ALLOCATED:  return "ALLOCATED";
        case StudentStatus::VACATED:    return "VACATED";
        case StudentStatus::CANCELLED:  return "CANCELLED";
        default:                        return "UNKNOWN";
    }
}

inline StudentStatus stringToStatus(const std::string& str) {
    if (str == "REGISTERED") return StudentStatus::REGISTERED;
    if (str == "WAITING")    return StudentStatus::WAITING;
    if (str == "ALLOCATED")  return StudentStatus::ALLOCATED;
    if (str == "VACATED")    return StudentStatus::VACATED;
    if (str == "CANCELLED")  return StudentStatus::CANCELLED;
    return StudentStatus::REGISTERED;
}

/**
 * @brief Student Record Entity.
 */
struct Student {
    std::string id;
    std::string name;
    int age = 0;
    std::string gender;       // "Male", "Female", "Other"
    std::string course;       // "CSE", "ECE", "ME", etc.
    int year = 1;             // 1 to 4
    float cgpa = 0.0f;        // 0.0 to 10.0
    float distanceKm = 0.0f;  // hometown distance
    bool isSpecialCategory = false; // quota/pwd/sports
    float priorityScore = 0.0f;     // calculated composite score

    std::string phone;
    std::string email;
    std::string guardian;
    std::string city;
    std::string bloodGroup;

    StudentStatus status = StudentStatus::REGISTERED;
    std::string allocatedRoomId = "";
    std::string allocatedDate = "";

    Student() = default;

    Student(const std::string& sId, const std::string& sName, int sAge,
            const std::string& sGender, const std::string& sCourse, int sYear,
            float sCgpa, float sDist = 0.0f, bool sSpecial = false,
            const std::string& sPhone = "", const std::string& sEmail = "",
            const std::string& sGuardian = "", const std::string& sCity = "",
            const std::string& sBlood = "")
        : id(sId), name(sName), age(sAge), gender(sGender), course(sCourse),
          year(sYear), cgpa(sCgpa), distanceKm(sDist), isSpecialCategory(sSpecial),
          phone(sPhone), email(sEmail), guardian(sGuardian), city(sCity), bloodGroup(sBlood),
          status(StudentStatus::REGISTERED) {
        calculatePriority();
    }

    // Calculates composite priority score:
    // Senior years, higher CGPA, greater travel distance, and special categories yield higher priority.
    void calculatePriority() {
        priorityScore = (static_cast<float>(year) * 25.0f) +
                        (cgpa * 5.0f) +
                        (distanceKm * 0.02f) +
                        (isSpecialCategory ? 15.0f : 0.0f);
    }
};

/**
 * @brief Lightweight reference used in the Priority Heap.
 */
struct StudentPriorityRef {
    std::string studentId;
    float priorityScore = 0.0f;
    int registrationOrder = 0; // secondary tie-breaker

    bool operator==(const StudentPriorityRef& other) const {
        return studentId == other.studentId;
    }

    // Max-Heap Comparator: higher priorityScore comes first.
    // If scores are equal, lower registration order (registered earlier) comes first.
    bool operator<(const StudentPriorityRef& other) const {
        if (priorityScore != other.priorityScore) {
            return priorityScore < other.priorityScore;
        }
        return registrationOrder > other.registrationOrder;
    }

    bool operator>(const StudentPriorityRef& other) const {
        if (priorityScore != other.priorityScore) {
            return priorityScore > other.priorityScore;
        }
        return registrationOrder < other.registrationOrder;
    }
};

} // namespace hostel
