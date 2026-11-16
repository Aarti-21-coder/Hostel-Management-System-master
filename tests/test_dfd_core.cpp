#include <iostream>
#include <cassert>
#include <string>
#include "core/HostelSystem.hpp"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[TEST FAILED] " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (0)

#define TEST_CASE(name) \
    std::cout << "Running Test: " << name << " ... " << std::flush

#define TEST_PASS() \
    std::cout << "[PASSED]" << std::endl

void testDuplicateStudentId() {
    TEST_CASE("Duplicate Student ID Rejection");
    hostel::HostelSystem sys;
    std::string err;

    hostel::Student s1("STU001", "Aarav Sharma", 20, "Male", "CSE", 2, 8.5f);
    bool ok1 = sys.registerStudent(s1, err);
    TEST_ASSERT(ok1, "First registration of STU001 must succeed");

    // Try registering duplicate ID
    hostel::Student s2("STU001", "Aarav Clone", 21, "Male", "ECE", 3, 7.5f);
    bool ok2 = sys.registerStudent(s2, err);
    TEST_ASSERT(!ok2, "Duplicate student ID STU001 must be rejected");
    TEST_ASSERT(err.find("Duplicate Student ID") != std::string::npos, "Error message must report duplicate ID");

    TEST_PASS();
}

void testPriorityAndNormalQueueOrder() {
    TEST_CASE("Priority Queue Served Before Normal Queue");
    hostel::HostelSystem sys;
    std::string err;

    // Normal student: 1st year, CGPA 5.0 -> score = (1*25) + (5*5) = 50. Wait, year 1, cgpa 4.0 -> score = 25 + 20 = 45 (< 50)
    hostel::Student normalStudent("NORM01", "Normal Priority", 18, "Male", "ME", 1, 4.0f, 10.0f, false);
    bool okNorm = sys.registerStudent(normalStudent, err);
    TEST_ASSERT(okNorm, "Register normal student");
    TEST_ASSERT(sys.getNormalQueueSize() == 1, "Normal queue size must be 1");
    TEST_ASSERT(sys.getPriorityQueueSize() == 0, "Priority queue size must be 0");

    // High priority student: 4th year, CGPA 9.5 -> score = (4*25) + (9.5*5) = 147.5 (>= 50)
    hostel::Student prioStudent("PRIO01", "High Priority", 21, "Male", "CSE", 4, 9.5f, 500.0f, true);
    bool okPrio = sys.registerStudent(prioStudent, err);
    TEST_ASSERT(okPrio, "Register priority student");
    TEST_ASSERT(sys.getPriorityQueueSize() == 1, "Priority queue size must be 1");

    // AllocateNext MUST allocate the priority student first even though normal student registered earlier!
    std::string allocStuId, allocRoomId;
    bool okAlloc = sys.allocateNext(allocStuId, allocRoomId, err);
    TEST_ASSERT(okAlloc, "Allocation must succeed");
    TEST_ASSERT(allocStuId == "PRIO01", "High priority student must be allocated before normal student");
    TEST_ASSERT(sys.getPriorityQueueSize() == 0, "Priority queue must now be empty");
    TEST_ASSERT(sys.getNormalQueueSize() == 1, "Normal student must still be in waiting queue");

    // Next allocation serves the normal student
    bool okAlloc2 = sys.allocateNext(allocStuId, allocRoomId, err);
    TEST_ASSERT(okAlloc2, "Second allocation must succeed");
    TEST_ASSERT(allocStuId == "NORM01", "Normal student must now be allocated");
    TEST_ASSERT(sys.getNormalQueueSize() == 0, "Normal queue must now be empty");

    TEST_PASS();
}

void testFullQueueHandling() {
    TEST_CASE("Full Queue Handling & Boundary Rejection");
    // Create system with tiny normal queue capacity = 2
    hostel::HostelSystem sys(2);
    std::string err;

    hostel::Student s1("NORM01", "Student 1", 18, "Male", "ME", 1, 4.0f);
    hostel::Student s2("NORM02", "Student 2", 18, "Male", "CE", 1, 4.0f);
    hostel::Student s3("NORM03", "Student 3", 18, "Male", "EE", 1, 4.0f);

    TEST_ASSERT(sys.registerStudent(s1, err), "S1 must fit in queue");
    TEST_ASSERT(sys.registerStudent(s2, err), "S2 must fit in queue (now full)");
    TEST_ASSERT(sys.getNormalQueueSize() == 2, "Queue size must be 2");

    // S3 should be rejected because normal queue capacity is 2
    bool ok3 = sys.registerStudent(s3, err);
    TEST_ASSERT(!ok3, "S3 must be rejected when circular queue is full");
    TEST_ASSERT(err.find("full") != std::string::npos, "Error message must report full queue");

    TEST_PASS();
}

void testFullRoomsHandling() {
    TEST_CASE("Full Rooms Handling (Student Retained in Queue)");
    // Create system with a custom single room with capacity 1
    hostel::HostelSystem sys;
    std::string err;

    // Allocate all rooms for Male students
    std::string allocStuId, allocRoomId;
    int countAllocated = 0;
    while (true) {
        hostel::Student s("FILL_" + std::to_string(countAllocated), "Filler", 20, "Male", "CSE", 3, 8.0f);
        sys.registerStudent(s, err);
        if (!sys.allocateNext(allocStuId, allocRoomId, err)) {
            // All male rooms are now full!
            break;
        }
        ++countAllocated;
    }

    TEST_ASSERT(countAllocated > 0, "Should have allocated several rooms until full");

    // Now try registering one more student
    hostel::Student overflowStudent("OVERFLOW01", "Overflow Student", 20, "Male", "CSE", 3, 8.0f);
    TEST_ASSERT(sys.registerStudent(overflowStudent, err), "Student registers and enters queue");
    TEST_ASSERT(sys.getStudent("OVERFLOW01")->status == hostel::StudentStatus::WAITING, "Student is in waiting status");

    // Allocation should fail gracefully with rooms full message
    bool allocResult = sys.allocateNext(allocStuId, allocRoomId, err);
    TEST_ASSERT(!allocResult, "Allocation must fail when rooms are full");
    TEST_ASSERT(err.find("all rooms full") != std::string::npos, "Error must indicate all rooms are full");

    // Student MUST remain in waiting queue!
    hostel::Student* st = sys.getStudent("OVERFLOW01");
    TEST_ASSERT(st != nullptr, "Student exists");
    TEST_ASSERT(st->status == hostel::StudentStatus::WAITING, "Student status remains WAITING");

    TEST_PASS();
}

void testVacateAndAutoReallocation() {
    TEST_CASE("Vacate with Immediate Auto-Reallocation");
    hostel::HostelSystem sys;
    std::string err;

    // Student A allocated to a room
    hostel::Student stuA("STU_A", "Student Alpha", 20, "Male", "CSE", 3, 8.0f);
    sys.registerStudent(stuA, err);
    std::string allocA, roomA;
    sys.allocateNext(allocA, roomA, err);
    TEST_ASSERT(allocA == "STU_A", "Student A allocated");

    // Student B waiting in priority queue
    hostel::Student stuB("STU_B", "Student Beta", 21, "Male", "ECE", 4, 9.0f);
    sys.registerStudent(stuB, err);
    TEST_ASSERT(sys.getPriorityQueueSize() > 0, "Student B in priority waiting list");

    // Student C waiting in normal queue
    hostel::Student stuC("STU_C", "Student Gamma", 18, "Male", "ME", 1, 4.0f);
    sys.registerStudent(stuC, err);
    TEST_ASSERT(sys.getNormalQueueSize() > 0, "Student C in normal waiting list");

    // When Student A vacates roomA, Student B (highest priority) should be AUTO-ALLOCATED into roomA!
    std::string autoAllocId;
    bool okVac = sys.vacateStudent("STU_A", err, &autoAllocId);
    TEST_ASSERT(okVac, "Vacate must succeed");
    TEST_ASSERT(autoAllocId == "STU_B", "Auto-allocation must select highest priority waiting student (STU_B)");

    hostel::Student* stB = sys.getStudent("STU_B");
    TEST_ASSERT(stB->status == hostel::StudentStatus::ALLOCATED, "Student B must now be ALLOCATED");
    TEST_ASSERT(stB->allocatedRoomId == roomA, "Student B must be in roomA");

    hostel::Student* stA = sys.getStudent("STU_A");
    TEST_ASSERT(stA->status == hostel::StudentStatus::VACATED, "Student A must be VACATED");

    TEST_PASS();
}

void testTransferConsistency() {
    TEST_CASE("Room Transfer Cross-Store Consistency");
    hostel::HostelSystem sys;
    std::string err;

    hostel::Student s1("TRANS01", "Transfer Candidate", 20, "Male", "CSE", 3, 8.0f);
    sys.registerStudent(s1, err);
    std::string allocId, r1Id;
    sys.allocateNext(allocId, r1Id, err);

    hostel::Room* r1 = sys.getRoom(r1Id);
    int r1OccBefore = r1->occupiedCount;

    // Find another available male room
    std::string r2Id = "";
    for (size_t i = 0; i < sys.getAllRooms().size(); ++i) {
        auto rm = sys.getAllRooms()[i];
        if (rm->id != r1Id && rm->canAccommodate("Male")) {
            r2Id = rm->id;
            break;
        }
    }
    TEST_ASSERT(!r2Id.empty(), "Found destination room");
    hostel::Room* r2 = sys.getRoom(r2Id);
    int r2OccBefore = r2->occupiedCount;

    // Transfer student
    bool okTrans = sys.transferStudent("TRANS01", r2Id, err);
    TEST_ASSERT(okTrans, "Transfer must succeed");

    // Check consistency across student, old room, new room, and allocated list
    hostel::Student* st = sys.getStudent("TRANS01");
    TEST_ASSERT(st->allocatedRoomId == r2Id, "Student allocatedRoomId must be updated to r2Id");
    TEST_ASSERT(r1->occupiedCount == r1OccBefore - 1, "Old room occupancy must be decremented");
    TEST_ASSERT(r2->occupiedCount == r2OccBefore + 1, "New room occupancy must be incremented");

    // Check allocated list node
    bool foundInList = false;
    sys.getAllocatedList().forEach([&](const hostel::AllocationRecord& rec) {
        if (rec.studentId == "TRANS01") {
            TEST_ASSERT(rec.roomId == r2Id, "Allocated list record must reflect new room ID");
            foundInList = true;
        }
    });
    TEST_ASSERT(foundInList, "Student must exist in allocated list");

    TEST_PASS();
}

void testCancellationConsistency() {
    TEST_CASE("Cancellation Consistency (Waiting vs Allocated)");
    hostel::HostelSystem sys;
    std::string err;

    // 1. Cancel while in WAITING queue
    hostel::Student sWait("CANCEL_WAIT", "Waiting Student", 19, "Male", "CSE", 1, 4.0f);
    sys.registerStudent(sWait, err);
    TEST_ASSERT(sys.getNormalQueueSize() == 1, "Student in normal queue");

    bool okCancelWait = sys.cancelRegistration("CANCEL_WAIT", err);
    TEST_ASSERT(okCancelWait, "Cancellation of waiting student must succeed");
    TEST_ASSERT(sys.getNormalQueueSize() == 0, "Student must be removed from normal queue");
    TEST_ASSERT(sys.getStudent("CANCEL_WAIT")->status == hostel::StudentStatus::CANCELLED, "Status must be CANCELLED");

    // 2. Cancel while ALLOCATED
    hostel::Student sAlloc("CANCEL_ALLOC", "Allocated Student", 20, "Male", "CSE", 3, 8.0f);
    sys.registerStudent(sAlloc, err);
    std::string aId, rId;
    sys.allocateNext(aId, rId, err);
    hostel::Room* rm = sys.getRoom(rId);
    int occBefore = rm->occupiedCount;

    bool okCancelAlloc = sys.cancelRegistration("CANCEL_ALLOC", err);
    TEST_ASSERT(okCancelAlloc, "Cancellation of allocated student must succeed");
    TEST_ASSERT(sys.getStudent("CANCEL_ALLOC")->status == hostel::StudentStatus::CANCELLED, "Status must be CANCELLED");
    TEST_ASSERT(rm->occupiedCount == occBefore - 1, "Bed must be freed upon cancellation");

    TEST_PASS();
}

void testUndoRedoOperations() {
    TEST_CASE("Undo and Redo Transaction Stack Operations");
    hostel::HostelSystem sys;
    std::string err;
    std::string msg;

    hostel::Student s1("UNDO01", "Undo Subject", 20, "Male", "CSE", 3, 8.0f);
    sys.registerStudent(s1, err);

    std::string aId, rId;
    sys.allocateNext(aId, rId, err);
    TEST_ASSERT(sys.getStudent("UNDO01")->status == hostel::StudentStatus::ALLOCATED, "Student allocated");
    hostel::Room* rm = sys.getRoom(rId);
    int occAllocated = rm->occupiedCount;

    // Test UNDO of Allocation
    TEST_ASSERT(sys.canUndo(), "Undo stack must have operations");
    bool okUndo = sys.undo(msg);
    TEST_ASSERT(okUndo, "Undo must succeed");
    TEST_ASSERT(sys.getStudent("UNDO01")->status == hostel::StudentStatus::WAITING, "Student status rolled back to WAITING");
    TEST_ASSERT(rm->occupiedCount == occAllocated - 1, "Room occupancy rolled back");

    // Test REDO of Allocation
    TEST_ASSERT(sys.canRedo(), "Redo stack must have operations");
    bool okRedo = sys.redo(msg);
    TEST_ASSERT(okRedo, "Redo must succeed");
    TEST_ASSERT(sys.getStudent("UNDO01")->status == hostel::StudentStatus::ALLOCATED, "Student status re-applied to ALLOCATED");
    TEST_ASSERT(rm->occupiedCount == occAllocated, "Room occupancy restored");

    TEST_PASS();
}

void testSortingReports() {
    TEST_CASE("DFD 4.0 Reports with QuickSort, MergeSort, HeapSort");
    hostel::HostelSystem sys;
    std::string err;

    hostel::Student s1("STU10", "Zack", 20, "Male", "CSE", 1, 6.0f);
    hostel::Student s2("STU02", "Alice", 21, "Female", "ECE", 4, 9.5f);
    hostel::Student s3("STU05", "Bob", 19, "Male", "ME", 2, 7.5f);

    sys.registerStudent(s1, err);
    sys.registerStudent(s2, err);
    sys.registerStudent(s3, err);

    dsa::SortMetrics metricsQ, metricsM, metricsH;
    double tQ = 0, tM = 0, tH = 0;

    // Sort by Name Ascending using QuickSort
    auto repQ = sys.getStudentReport(hostel::SortField::NAME, true, hostel::SortAlgorithmType::QUICK_SORT, metricsQ, tQ);
    TEST_ASSERT(repQ.size() >= 3, "Report size check");
    TEST_ASSERT(repQ[0].name == "Alice", "First alphabetically must be Alice");
    TEST_ASSERT(repQ[repQ.size() - 1].name == "Zack", "Last alphabetically must be Zack");

    // Sort by CGPA Descending using MergeSort
    auto repM = sys.getStudentReport(hostel::SortField::CGPA, false, hostel::SortAlgorithmType::MERGE_SORT, metricsM, tM);
    TEST_ASSERT(repM[0].name == "Alice", "Highest CGPA must be Alice");

    // Sort by Priority Descending using HeapSort
    auto repH = sys.getStudentReport(hostel::SortField::PRIORITY, false, hostel::SortAlgorithmType::HEAP_SORT, metricsH, tH);
    TEST_ASSERT(repH[0].name == "Alice", "Highest Priority must be Alice");

    TEST_PASS();
}

int main() {
    std::cout << "\n==========================================" << std::endl;
    std::cout << "HOSTEL MANAGEMENT SYSTEM - DFD CORE TESTS" << std::endl;
    std::cout << "==========================================\n" << std::endl;

    testDuplicateStudentId();
    testPriorityAndNormalQueueOrder();
    testFullQueueHandling();
    testFullRoomsHandling();
    testVacateAndAutoReallocation();
    testTransferConsistency();
    testCancellationConsistency();
    testUndoRedoOperations();
    testSortingReports();

    std::cout << "\n==========================================" << std::endl;
    std::cout << "ALL 9 DFD CORE TEST SUITES PASSED (100%)" << std::endl;
    std::cout << "==========================================\n" << std::endl;

    return 0;
}
