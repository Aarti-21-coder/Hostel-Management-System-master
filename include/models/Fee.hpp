#pragma once

#include <string>

namespace hostel {

struct FeeRecord {
    std::string recordId;
    std::string studentId;
    float baseAmount = 5000.0f;
    float lateFine = 0.0f;
    float paidAmount = 0.0f;
    bool isPaid = false;
    std::string dueDate;
    std::string paidDate;
    std::string receiptNumber;

    FeeRecord() = default;

    FeeRecord(std::string rId, std::string sId, float base, float fine,
              std::string due, bool paid = false)
        : recordId(std::move(rId)), studentId(std::move(sId)),
          baseAmount(base), lateFine(fine), isPaid(paid), dueDate(std::move(due)) {}

    [[nodiscard]] float totalDue() const noexcept {
        return isPaid ? 0.0f : (baseAmount + lateFine - paidAmount);
    }
};

} // namespace hostel
