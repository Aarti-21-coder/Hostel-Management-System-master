#pragma once

#include <string>

namespace hostel {

struct VisitorPass {
    std::string passId;
    std::string visitorName;
    std::string visitorPhone;
    std::string studentId;
    std::string relationship; // "Parent", "Sibling", "Guardian", "Friend"
    std::string visitDate;
    std::string checkInTime;
    std::string checkOutTime;
    bool isBlacklisted = false;
    std::string purpose;

    VisitorPass() = default;

    VisitorPass(std::string pId, std::string vName, std::string phone,
                std::string sId, std::string rel, std::string date,
                std::string inTime, std::string purposeStr = "")
        : passId(std::move(pId)), visitorName(std::move(vName)),
          visitorPhone(std::move(phone)), studentId(std::move(sId)),
          relationship(std::move(rel)), visitDate(std::move(date)),
          checkInTime(std::move(inTime)), purpose(std::move(purposeStr)) {}
};

struct LeaveRequest {
    std::string requestId;
    std::string studentId;
    int startDay = 0; // Epoch day or day index
    int endDay = 0;
    std::string startDateStr;
    std::string endDateStr;
    std::string reason;
    std::string status = "PENDING"; // "PENDING", "APPROVED", "REJECTED"
    bool lateReturn = false;

    LeaveRequest() = default;

    LeaveRequest(std::string rId, std::string sId, int start, int end,
                 std::string sDate, std::string eDate, std::string rsn)
        : requestId(std::move(rId)), studentId(std::move(sId)),
          startDay(start), endDay(end), startDateStr(std::move(sDate)),
          endDateStr(std::move(eDate)), reason(std::move(rsn)) {}
};

struct MessDayMenu {
    std::string day; // "Monday", etc.
    std::string breakfast;
    std::string lunch;
    std::string snacks;
    std::string dinner;
    float dailyRate = 120.0f;
};

struct Notice {
    std::string id;
    std::string title;
    std::string content;
    std::string author;
    std::string date;
    std::string category; // "General", "Mess", "Curfew", "Maintenance"
    bool isUrgent = false;

    Notice() = default;
    Notice(std::string nId, std::string t, std::string c, std::string auth,
           std::string d, std::string cat, bool urgent = false)
        : id(std::move(nId)), title(std::move(t)), content(std::move(c)),
          author(std::move(auth)), date(std::move(d)), category(std::move(cat)),
          isUrgent(urgent) {}
};

} // namespace hostel
