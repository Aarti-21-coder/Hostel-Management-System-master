#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <memory>

#include "core/HostelSystem.hpp"
#include "core/AuthManager.hpp"
#include "core/DataGenerator.hpp"
#include "server/HttpServer.hpp"

void printHeader(const std::string& title) {
    std::cout << "\n======================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "======================================================\n";
}

void pauseConsole() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void handleRegistration(hostel::HostelSystem& sys) {
    printHeader("DFD 2.0: Student Registration");
    std::string id, name, gender, course, phone, email, city;
    int age = 0, year = 1;
    float cgpa = 0.0f, dist = 0.0f;
    char specChar = 'n';

    std::cout << "Enter Student ID (e.g. STU101): ";
    std::cin >> id;
    std::cin.ignore();

    std::cout << "Enter Full Name: ";
    std::getline(std::cin, name);

    std::cout << "Enter Age: ";
    std::cin >> age;

    std::cout << "Enter Gender (Male/Female/Other): ";
    std::cin >> gender;

    std::cout << "Enter Course (e.g. CSE/ECE/ME): ";
    std::cin >> course;

    std::cout << "Enter Year of Study (1-4): ";
    std::cin >> year;

    std::cout << "Enter CGPA (0.0 - 10.0): ";
    std::cin >> cgpa;

    std::cout << "Enter Hometown Distance in Km: ";
    std::cin >> dist;

    std::cout << "Special Category (Sports/Differently-abled/Staff ward) [y/n]: ";
    std::cin >> specChar;
    bool isSpecial = (specChar == 'y' || specChar == 'Y');

    std::cin.ignore();
    std::cout << "Enter Phone Number: ";
    std::getline(std::cin, phone);

    std::cout << "Enter Email Address: ";
    std::getline(std::cin, email);

    std::cout << "Enter City: ";
    std::getline(std::cin, city);

    hostel::Student st(id, name, age, gender, course, year, cgpa, dist, isSpecial, phone, email, "", city, "");
    std::string err;
    if (sys.registerStudent(st, err)) {
        hostel::Student* registered = sys.getStudent(id);
        std::cout << "\n[SUCCESS] Student successfully registered!\n";
        std::cout << " -> Computed Priority Score: " << std::fixed << std::setprecision(2) << registered->priorityScore << "\n";
        if (registered->priorityScore >= 50.0f) {
            std::cout << " -> Routing: High Priority (Score >= 50.0) -> Pushed to Binary Max-Heap Priority Queue\n";
        } else {
            std::cout << " -> Routing: Normal Priority (Score < 50.0) -> Enqueued to Circular FIFO Queue\n";
        }
        std::cout << " -> Status: WAITING\n";
    } else {
        std::cout << "\n[ERROR] Registration failed: " << err << "\n";
    }
}

void handleViewWaitingLists(const hostel::HostelSystem& sys) {
    printHeader("Waiting Lists (Priority Max-Heap & Normal Circular Queue)");

    auto prioSnap = sys.getPriorityQueueSnapshot();
    std::cout << "\n[1] PRIORITY WAITING LIST (Binary Max-Heap - Always Served First):\n";
    std::cout << "Total Waiting: " << prioSnap.size() << "\n";
    std::cout << "------------------------------------------------------\n";
    std::cout << std::left << std::setw(12) << "Student ID"
              << std::setw(22) << "Name"
              << std::setw(15) << "Priority Score"
              << "Gender\n";
    std::cout << "------------------------------------------------------\n";
    for (size_t i = 0; i < prioSnap.size(); ++i) {
        const auto* st = sys.getStudent(prioSnap[i].studentId);
        if (st) {
            std::cout << std::left << std::setw(12) << st->id
                      << std::setw(22) << st->name
                      << std::setw(15) << std::fixed << std::setprecision(2) << st->priorityScore
                      << st->gender << "\n";
        }
    }

    auto normSnap = sys.getNormalQueueSnapshot();
    std::cout << "\n[2] NORMAL WAITING LIST (Circular FIFO Queue):\n";
    std::cout << "Total Waiting: " << normSnap.size() << "\n";
    std::cout << "------------------------------------------------------\n";
    std::cout << std::left << std::setw(12) << "Student ID"
              << std::setw(22) << "Name"
              << std::setw(15) << "Priority Score"
              << "Gender\n";
    std::cout << "------------------------------------------------------\n";
    for (size_t i = 0; i < normSnap.size(); ++i) {
        const auto* st = sys.getStudent(normSnap[i]);
        if (st) {
            std::cout << std::left << std::setw(12) << st->id
                      << std::setw(22) << st->name
                      << std::setw(15) << std::fixed << std::setprecision(2) << st->priorityScore
                      << st->gender << "\n";
        }
    }
}

void handleAllocation(hostel::HostelSystem& sys) {
    printHeader("DFD 3.0: Room Allocation & Availability Manager");
    std::cout << "Serving next student from waiting lists...\n";
    std::cout << "Rule: Priority Queue is served first; Normal Queue FIFO served next.\n\n";

    std::string sId, rId, err;
    if (sys.allocateNext(sId, rId, err)) {
        const auto* st = sys.getStudent(sId);
        const auto* rm = sys.getRoom(rId);
        std::cout << "[ALLOCATED SUCCESS]\n";
        std::cout << " -> Student: " << st->name << " (" << st->id << "), CGPA: " << st->cgpa << "\n";
        std::cout << " -> Room: " << rm->id << " (" << rm->block << ", Floor " << rm->floor << ")\n";
        std::cout << " -> Room Type: " << rm->roomType << (rm->isAC ? " [AC]" : " [Non-AC]") << "\n";
        std::cout << " -> Room Bed Occupancy: " << rm->occupiedCount << "/" << rm->capacity << "\n";
        std::cout << " -> Status: ALLOCATED\n";
    } else {
        std::cout << "[ALLOCATION FAILED]: " << err << "\n";
    }
}

void handleViewAllocated(const hostel::HostelSystem& sys) {
    printHeader("Allocated Students DB (Doubly Linked List Store)");
    const auto& list = sys.getAllocatedList();
    std::cout << "Total Allocated Students: " << list.size() << "\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(12) << "Student ID"
              << std::setw(24) << "Student Name"
              << std::setw(12) << "Room ID"
              << std::setw(14) << "Date"
              << "Bed #\n";
    std::cout << "----------------------------------------------------------------------\n";
    list.forEach([](const hostel::AllocationRecord& rec) {
        std::cout << std::left << std::setw(12) << rec.studentId
                  << std::setw(24) << rec.studentName
                  << std::setw(12) << rec.roomId
                  << std::setw(14) << rec.allocatedDate
                  << rec.bedIndex << "\n";
    });
}

void handleVacate(hostel::HostelSystem& sys) {
    printHeader("Vacate Room (With Immediate Auto-Reallocation)");
    std::string sId, err, autoId;
    std::cout << "Enter Student ID to vacate: ";
    std::cin >> sId;

    if (sys.vacateStudent(sId, err, &autoId)) {
        std::cout << "\n[SUCCESS] Student " << sId << " has vacated their room.\n";
        if (!autoId.empty()) {
            const auto* autoSt = sys.getStudent(autoId);
            std::cout << "\n[AUTO-REALLOCATION TRIGGERED!]\n";
            std::cout << " -> Freed bed was immediately auto-allocated to waiting student:\n";
            std::cout << "    Name: " << autoSt->name << " (ID: " << autoSt->id << ")\n";
            std::cout << "    Room: " << autoSt->allocatedRoomId << "\n";
            std::cout << "    Priority Score: " << autoSt->priorityScore << "\n";
        } else {
            std::cout << " -> No waiting students currently match this room's gender/criteria.\n";
        }
    } else {
        std::cout << "\n[ERROR]: " << err << "\n";
    }
}

void handleTransfer(hostel::HostelSystem& sys) {
    printHeader("Transfer Student Room");
    std::string sId, rId, err;
    std::cout << "Enter Student ID: ";
    std::cin >> sId;
    std::cout << "Enter Destination Room ID (e.g. A-102): ";
    std::cin >> rId;

    if (sys.transferStudent(sId, rId, err)) {
        std::cout << "\n[SUCCESS] Student " << sId << " successfully transferred to room " << rId << "\n";
        std::cout << " -> Cross-structure consistency: old room freed, new room allocated, DoublyLinkedList updated.\n";
    } else {
        std::cout << "\n[ERROR]: " << err << "\n";
    }
}

void handleCancel(hostel::HostelSystem& sys) {
    printHeader("Cancel Student Registration");
    std::string sId, err;
    std::cout << "Enter Student ID to cancel: ";
    std::cin >> sId;

    if (sys.cancelRegistration(sId, err)) {
        std::cout << "\n[SUCCESS] Registration for student " << sId << " cancelled.\n";
        std::cout << " -> Removed from waiting lists / room vacated if allocated.\n";
    } else {
        std::cout << "\n[ERROR]: " << err << "\n";
    }
}

void handleUndo(hostel::HostelSystem& sys) {
    std::string msg;
    if (sys.undo(msg)) {
        std::cout << "\n[UNDO SUCCESS]: " << msg << "\n";
    } else {
        std::cout << "\n[UNDO]: " << msg << "\n";
    }
}

void handleRedo(hostel::HostelSystem& sys) {
    std::string msg;
    if (sys.redo(msg)) {
        std::cout << "\n[REDO SUCCESS]: " << msg << "\n";
    } else {
        std::cout << "\n[REDO]: " << msg << "\n";
    }
}

void handleReports(const hostel::HostelSystem& sys) {
    printHeader("DFD 4.0: View Reports with Custom Sorts");
    std::cout << "Select Sort Field:\n";
    std::cout << " [1] Name (A-Z)\n [2] Student ID\n [3] Year of Study\n [4] Priority Score\n [5] CGPA\nChoice: ";
    int fChoice = 1;
    std::cin >> fChoice;
    hostel::SortField field = hostel::SortField::NAME;
    if (fChoice == 2) field = hostel::SortField::ID;
    else if (fChoice == 3) field = hostel::SortField::YEAR;
    else if (fChoice == 4) field = hostel::SortField::PRIORITY;
    else if (fChoice == 5) field = hostel::SortField::CGPA;

    std::cout << "\nSelect Custom Sorting Algorithm:\n";
    std::cout << " [1] QuickSort (Median-of-three, in-place)\n [2] MergeSort (Stable divide-and-conquer)\n [3] HeapSort (In-place binary heap)\nChoice: ";
    int aChoice = 1;
    std::cin >> aChoice;
    hostel::SortAlgorithmType algo = hostel::SortAlgorithmType::QUICK_SORT;
    if (aChoice == 2) algo = hostel::SortAlgorithmType::MERGE_SORT;
    else if (aChoice == 3) algo = hostel::SortAlgorithmType::HEAP_SORT;

    dsa::SortMetrics metrics;
    double elapsedUs = 0.0;
    auto sorted = sys.getStudentReport(field, true, algo, metrics, elapsedUs);

    std::cout << "\n-------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(12) << "ID"
              << std::setw(22) << "Name"
              << std::setw(8) << "Year"
              << std::setw(8) << "CGPA"
              << std::setw(12) << "Priority"
              << std::setw(14) << "Status"
              << "Room\n";
    std::cout << "-------------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < sorted.size(); ++i) {
        std::cout << std::left << std::setw(12) << sorted[i].id
                  << std::setw(22) << sorted[i].name
                  << std::setw(8) << sorted[i].year
                  << std::setw(8) << std::fixed << std::setprecision(1) << sorted[i].cgpa
                  << std::setw(12) << std::fixed << std::setprecision(2) << sorted[i].priorityScore
                  << std::setw(14) << hostel::statusToString(sorted[i].status)
                  << sorted[i].allocatedRoomId << "\n";
    }

    std::cout << "\n[DSA Performance Metrics]:\n";
    std::cout << " -> Records Sorted: " << sorted.size() << "\n";
    std::cout << " -> Execution Time: " << std::fixed << std::setprecision(2) << elapsedUs << " microseconds\n";
    std::cout << " -> Element Comparisons: " << metrics.comparisons << "\n";
    std::cout << " -> Element Swaps/Moves: " << metrics.swaps << "\n";
}

void handleViewRooms(const hostel::HostelSystem& sys) {
    printHeader("Room Inventory & Availability Status");
    auto rooms = sys.getRoomReport(false);
    std::cout << "Total Rooms: " << rooms.size() << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(10) << "Room ID"
              << std::setw(12) << "Block"
              << std::setw(8) << "Floor"
              << std::setw(10) << "Type"
              << std::setw(6) << "AC"
              << std::setw(12) << "Allowed"
              << std::setw(12) << "Occupancy"
              << std::setw(14) << "Status"
              << "Monthly Fee\n";
    std::cout << "---------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < rooms.size(); ++i) {
        std::string occ = std::to_string(rooms[i].occupiedCount) + "/" + std::to_string(rooms[i].capacity);
        std::cout << std::left << std::setw(10) << rooms[i].id
                  << std::setw(12) << rooms[i].block
                  << std::setw(8) << rooms[i].floor
                  << std::setw(10) << rooms[i].roomType
                  << std::setw(6) << (rooms[i].isAC ? "Yes" : "No")
                  << std::setw(12) << rooms[i].allowedGender
                  << std::setw(12) << occ
                  << std::setw(14) << hostel::roomStatusToString(rooms[i].status)
                  << "$" << std::fixed << std::setprecision(0) << rooms[i].monthlyFee << "\n";
    }
}

void seedSampleData(hostel::HostelSystem& sys) {
    std::string err;
    hostel::Student s1("STU101", "Aarav Sharma", 20, "Male", "CSE", 3, 9.2f, 450.0f, true);
    hostel::Student s2("STU102", "Priya Patel", 19, "Female", "ECE", 2, 8.8f, 220.0f, false);
    hostel::Student s3("STU103", "Rohan Verma", 21, "Male", "ME", 4, 7.5f, 600.0f, false);
    hostel::Student s4("STU104", "Ananya Iyer", 18, "Female", "CSE", 1, 9.6f, 150.0f, false);
    hostel::Student s5("STU105", "Kabir Mehta", 18, "Male", "CE", 1, 6.0f, 40.0f, false);

    sys.registerStudent(s1, err);
    sys.registerStudent(s2, err);
    sys.registerStudent(s3, err);
    sys.registerStudent(s4, err);
    sys.registerStudent(s5, err);
}

int main(int argc, char* argv[]) {
    auto sys = std::make_shared<hostel::HostelSystem>();
    auto auth = std::make_shared<hostel::AuthManager>();
    seedSampleData(*sys);

    bool consoleMode = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--console" || arg == "-c") {
            consoleMode = true;
        }
    }

    if (consoleMode) {
        while (true) {
            std::cout << "\n";
            std::cout << "=================================================================\n";
            std::cout << "         HOSTEL MANAGEMENT SYSTEM - DSA CORE CONSOLE             \n";
            std::cout << "             (Data Flow Diagram Level 0/1/2 Demo)                \n";
            std::cout << "=================================================================\n";
            std::cout << " [1] Student Registration (DFD 2.0: Vector + Hash + Queues)\n";
            std::cout << " [2] View Waiting Lists (Priority Heap & Circular Queue)\n";
            std::cout << " [3] Room Allocation (DFD 3.0: Priority Served First)\n";
            std::cout << " [4] View Allocated Students DB (Doubly Linked List)\n";
            std::cout << " [5] Vacate Room (Demonstrates Immediate Auto-Reallocation)\n";
            std::cout << " [6] Transfer Student Room (Atomic Cross-Store Update)\n";
            std::cout << " [7] Cancel Student Registration (Consistent Removal)\n";
            std::cout << " [8] Undo Last Admin Action (Stack 1)\n";
            std::cout << " [9] Redo Last Admin Action (Stack 2)\n";
            std::cout << " [10] View Reports (DFD 4.0: Custom Quick/Merge/HeapSort)\n";
            std::cout << " [11] View Room Inventory & Availability\n";
            std::cout << " [0] Exit\n";
            std::cout << "=================================================================\n";
            std::cout << "Select an option [0-11]: ";

            int choice = -1;
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            switch (choice) {
                case 1:  handleRegistration(*sys); break;
                case 2:  handleViewWaitingLists(*sys); break;
                case 3:  handleAllocation(*sys); break;
                case 4:  handleViewAllocated(*sys); break;
                case 5:  handleVacate(*sys); break;
                case 6:  handleTransfer(*sys); break;
                case 7:  handleCancel(*sys); break;
                case 8:  handleUndo(*sys); break;
                case 9:  handleRedo(*sys); break;
                case 10: handleReports(*sys); break;
                case 11: handleViewRooms(*sys); break;
                case 0:
                    std::cout << "\nExiting Hostel Management System. Goodbye!\n";
                    return 0;
                default:
                    std::cout << "\nInvalid choice! Please select 0-11.\n";
                    break;
            }
            pauseConsole();
        }
    } else {
        // Web mode (Default)
        hostel::HttpServer webServer(sys, auth, 8080, "frontend");
        webServer.start();
    }

    return 0;
}
