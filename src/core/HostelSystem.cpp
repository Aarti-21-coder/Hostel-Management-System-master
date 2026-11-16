#include "core/HostelSystem.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace hostel {

HostelSystem::HostelSystem(size_t normalQueueCapacity)
    : normalQueue_(normalQueueCapacity) {
    seedDefaultRooms();
}

std::string HostelSystem::getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm ltm;
#if defined(_MSC_VER) || defined(__MINGW32__)
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&ltm, "%Y-%m-%d");
    return oss.str();
}

void HostelSystem::seedDefaultRooms() {
    std::string blocks[] = {"A", "B", "C", "D", "E"};
    for (const auto& blk : blocks) {
        std::string blockName = "Block " + blk;
        std::string gender = (blk == "A" || blk == "B") ? "Male" : "Female";
        for (int fl = 1; fl <= 3; ++fl) {
            for (int r = 1; r <= 4; ++r) {
                std::string rId = blk + "-" + std::to_string(fl) + (r < 10 ? "0" : "") + std::to_string(r);
                int cap = (r % 3 == 0) ? 3 : ((r % 2 == 0) ? 2 : 1);
                std::string rType = (cap == 1) ? "Single" : ((cap == 2) ? "Double" : "Triple");
                bool ac = (fl >= 2);
                float fee = (cap == 1 ? 9000.0f : (cap == 2 ? 6500.0f : 4500.0f)) + (ac ? 2000.0f : 0.0f);
                Room rm(rId, blockName, fl, cap, rType, ac, gender, fee);
                std::string err;
                addRoom(rm, err);
            }
        }
    }
}

bool HostelSystem::addRoom(const Room& room, std::string& errorMsg) {
    if (room.id.empty()) {
        errorMsg = "Room ID cannot be empty.";
        return false;
    }
    if (roomMap_.contains(room.id)) {
        errorMsg = "Room ID '" + room.id + "' already exists.";
        return false;
    }

    auto roomPtr = std::make_shared<Room>(room);
    rooms_.push_back(roomPtr);
    roomMap_.insert(room.id, roomPtr.get());
    return true;
}

Room* HostelSystem::getRoom(const std::string& roomId) {
    Room** r = roomMap_.find(roomId);
    return r ? *r : nullptr;
}

const Room* HostelSystem::getRoom(const std::string& roomId) const {
    const Room* const* r = roomMap_.find(roomId);
    return r ? *r : nullptr;
}

Room* HostelSystem::findAvailableRoom(const std::string& gender, const std::string& preferredType) {
    for (size_t i = 0; i < rooms_.size(); ++i) {
        Room* r = rooms_[i].get();
        if (r && r->canAccommodate(gender)) {
            if (preferredType.empty() || r->roomType == preferredType) {
                return r;
            }
        }
    }
    // Fallback without type restriction if preferred type is full
    if (!preferredType.empty()) {
        for (size_t i = 0; i < rooms_.size(); ++i) {
            Room* r = rooms_[i].get();
            if (r && r->canAccommodate(gender)) {
                return r;
            }
        }
    }
    return nullptr;
}

bool HostelSystem::registerStudent(Student student, std::string& errorMsg) {
    if (student.id.empty()) {
        errorMsg = "Student ID cannot be empty.";
        return false;
    }
    if (student.name.empty()) {
        errorMsg = "Student name cannot be empty.";
        return false;
    }
    if (student.course.empty()) {
        errorMsg = "Course cannot be empty.";
        return false;
    }
    if (studentMap_.contains(student.id)) {
        errorMsg = "Duplicate Student ID: '" + student.id + "' is already registered.";
        return false;
    }

    student.calculatePriority();
    student.status = StudentStatus::WAITING;
    bool isHighPriority = (student.priorityScore >= 50.0f);

    // If normal priority, test queue capacity first
    if (!isHighPriority && normalQueue_.full()) {
        errorMsg = "Normal waiting list is full! Capacity limit reached.";
        return false;
    }

    auto sPtr = std::make_shared<Student>(student);
    students_.push_back(sPtr);
    studentMap_.insert(student.id, sPtr.get());

    if (isHighPriority) {
        priorityQueue_.push({student.id, student.priorityScore, ++registrationCounter_});
    } else {
        normalQueue_.enqueue(student.id);
    }

    undoStack_.push(std::make_shared<RegisterCommand>(student.id, isHighPriority));
    redoStack_.clear();
    return true;
}

Student* HostelSystem::getStudent(const std::string& studentId) {
    Student** s = studentMap_.find(studentId);
    return s ? *s : nullptr;
}

const Student* HostelSystem::getStudent(const std::string& studentId) const {
    const Student* const* s = studentMap_.find(studentId);
    return s ? *s : nullptr;
}

dsa::Vector<StudentPriorityRef> HostelSystem::getPriorityQueueSnapshot() const {
    dsa::Vector<StudentPriorityRef> snapshot;
    const StudentPriorityRef* raw = priorityQueue_.rawData();
    for (size_t i = 0; i < priorityQueue_.size(); ++i) {
        snapshot.push_back(raw[i]);
    }
    return snapshot;
}

dsa::Vector<std::string> HostelSystem::getNormalQueueSnapshot() const {
    dsa::Vector<std::string> snapshot;
    normalQueue_.forEach([&snapshot](const std::string& id) {
        snapshot.push_back(id);
    });
    return snapshot;
}

bool HostelSystem::allocateNext(std::string& allocatedStudentId, std::string& allocatedRoomId, std::string& errorMsg) {
    if (priorityQueue_.empty() && normalQueue_.empty()) {
        errorMsg = "Both priority and normal waiting lists are currently empty.";
        return false;
    }

    // 1. Priority Queue is ALWAYS served first
    if (!priorityQueue_.empty()) {
        StudentPriorityRef topRef = priorityQueue_.top();
        Student* student = getStudent(topRef.studentId);
        if (!student) {
            priorityQueue_.pop();
            errorMsg = "Invalid student reference in priority queue.";
            return false;
        }

        Room* room = findAvailableRoom(student->gender);
        if (!room) {
            errorMsg = "No available rooms matching student's requirements (all rooms full). Student remains in priority queue.";
            return false;
        }

        // Pop from queue and allocate
        priorityQueue_.pop();
        room->allocateBed();
        student->status = StudentStatus::ALLOCATED;
        student->allocatedRoomId = room->id;
        student->allocatedDate = getCurrentDate();

        allocatedList_.push_back(AllocationRecord(student->id, room->id, student->name, student->allocatedDate, room->occupiedCount));
        allocatedStudentId = student->id;
        allocatedRoomId = room->id;

        undoStack_.push(std::make_shared<AllocateCommand>(student->id, room->id, student->allocatedDate, true));
        redoStack_.clear();
        return true;
    }

    // 2. Normal Queue served next (FIFO)
    if (!normalQueue_.empty()) {
        std::string nextId = normalQueue_.front();
        Student* student = getStudent(nextId);
        if (!student) {
            normalQueue_.dequeue();
            errorMsg = "Invalid student reference in normal queue.";
            return false;
        }

        Room* room = findAvailableRoom(student->gender);
        if (!room) {
            errorMsg = "No available rooms matching student's requirements (all rooms full). Student remains in normal queue.";
            return false;
        }

        normalQueue_.dequeue();
        room->allocateBed();
        student->status = StudentStatus::ALLOCATED;
        student->allocatedRoomId = room->id;
        student->allocatedDate = getCurrentDate();

        allocatedList_.push_back(AllocationRecord(student->id, room->id, student->name, student->allocatedDate, room->occupiedCount));
        allocatedStudentId = student->id;
        allocatedRoomId = room->id;

        undoStack_.push(std::make_shared<AllocateCommand>(student->id, room->id, student->allocatedDate, false));
        redoStack_.clear();
        return true;
    }

    errorMsg = "No eligible students in waiting lists.";
    return false;
}

bool HostelSystem::allocateStudentToRoom(const std::string& studentId, const std::string& roomId, std::string& errorMsg) {
    Student* student = getStudent(studentId);
    if (!student) {
        errorMsg = "Student ID '" + studentId + "' not found.";
        return false;
    }
    if (student->status == StudentStatus::ALLOCATED) {
        errorMsg = "Student is already allocated to room " + student->allocatedRoomId;
        return false;
    }
    Room* room = getRoom(roomId);
    if (!room) {
        errorMsg = "Room ID '" + roomId + "' not found.";
        return false;
    }
    if (!room->canAccommodate(student->gender)) {
        errorMsg = "Room '" + roomId + "' is either full or cannot accommodate student.";
        return false;
    }

    // Remove from whichever waiting queue the student was in
    bool wasPri = (student->priorityScore >= 50.0f);
    if (wasPri) {
        priorityQueue_.removeIf([&studentId](const StudentPriorityRef& r) { return r.studentId == studentId; });
    } else {
        normalQueue_.remove(studentId);
    }

    room->allocateBed();
    student->status = StudentStatus::ALLOCATED;
    student->allocatedRoomId = room->id;
    student->allocatedDate = getCurrentDate();

    allocatedList_.push_back(AllocationRecord(student->id, room->id, student->name, student->allocatedDate, room->occupiedCount));
    undoStack_.push(std::make_shared<AllocateCommand>(student->id, room->id, student->allocatedDate, wasPri));
    redoStack_.clear();
    return true;
}

bool HostelSystem::vacateStudent(const std::string& studentId, std::string& errorMsg, std::string* autoAllocatedStudentId) {
    Student* student = getStudent(studentId);
    if (!student) {
        errorMsg = "Student ID '" + studentId + "' not found.";
        return false;
    }
    if (student->status != StudentStatus::ALLOCATED) {
        errorMsg = "Student '" + studentId + "' is not currently allocated to any room.";
        return false;
    }

    std::string freedRoomId = student->allocatedRoomId;
    Room* room = getRoom(freedRoomId);

    // Vacate student
    student->status = StudentStatus::VACATED;
    student->allocatedRoomId = "";

    if (room) {
        room->freeBed();
    }

    // O(1) removal from DoublyLinkedList
    allocatedList_.removeIf([&studentId](const AllocationRecord& rec) {
        return rec.studentId == studentId;
    });

    // AUTO-ALLOCATION TRIGGER:
    // Immediately allocate the freed bed to the next eligible student from Priority Queue first, then Normal Queue
    std::string autoId = "";
    if (room && room->isAvailable()) {
        // Try priority queue first
        if (!priorityQueue_.empty()) {
            StudentPriorityRef topRef = priorityQueue_.top();
            Student* cand = getStudent(topRef.studentId);
            if (cand && room->canAccommodate(cand->gender)) {
                priorityQueue_.pop();
                room->allocateBed();
                cand->status = StudentStatus::ALLOCATED;
                cand->allocatedRoomId = room->id;
                cand->allocatedDate = getCurrentDate();
                allocatedList_.push_back(AllocationRecord(cand->id, room->id, cand->name, cand->allocatedDate, room->occupiedCount));
                autoId = cand->id;
            }
        }
        // If not filled from priority queue, try normal queue
        if (autoId.empty() && !normalQueue_.empty()) {
            std::string candId = normalQueue_.front();
            Student* cand = getStudent(candId);
            if (cand && room->canAccommodate(cand->gender)) {
                normalQueue_.dequeue();
                room->allocateBed();
                cand->status = StudentStatus::ALLOCATED;
                cand->allocatedRoomId = room->id;
                cand->allocatedDate = getCurrentDate();
                allocatedList_.push_back(AllocationRecord(cand->id, room->id, cand->name, cand->allocatedDate, room->occupiedCount));
                autoId = cand->id;
            }
        }
    }

    if (autoAllocatedStudentId) {
        *autoAllocatedStudentId = autoId;
    }

    auto cmd = std::make_shared<VacateCommand>(studentId, freedRoomId, autoId);
    undoStack_.push(cmd);
    redoStack_.clear();
    return true;
}

bool HostelSystem::transferStudent(const std::string& studentId, const std::string& newRoomId, std::string& errorMsg) {
    Student* student = getStudent(studentId);
    if (!student) {
        errorMsg = "Student not found.";
        return false;
    }
    if (student->status != StudentStatus::ALLOCATED) {
        errorMsg = "Student is not currently allocated to a room.";
        return false;
    }
    if (student->allocatedRoomId == newRoomId) {
        errorMsg = "Student is already residing in room " + newRoomId;
        return false;
    }

    Room* newRoom = getRoom(newRoomId);
    if (!newRoom) {
        errorMsg = "Destination room '" + newRoomId + "' does not exist.";
        return false;
    }
    if (!newRoom->canAccommodate(student->gender)) {
        errorMsg = "Destination room '" + newRoomId + "' cannot accommodate this student (full or gender mismatch).";
        return false;
    }

    std::string oldRoomId = student->allocatedRoomId;
    Room* oldRoom = getRoom(oldRoomId);

    // Free bed in old room
    if (oldRoom) oldRoom->freeBed();

    // Allocate bed in new room
    newRoom->allocateBed();
    student->allocatedRoomId = newRoomId;

    // Update in DoublyLinkedList
    auto* node = allocatedList_.findIf([&studentId](const AllocationRecord& rec) {
        return rec.studentId == studentId;
    });
    if (node) {
        node->data.roomId = newRoomId;
        node->data.bedIndex = newRoom->occupiedCount;
    }

    undoStack_.push(std::make_shared<TransferCommand>(studentId, oldRoomId, newRoomId));
    redoStack_.clear();
    return true;
}

bool HostelSystem::cancelRegistration(const std::string& studentId, std::string& errorMsg) {
    Student* student = getStudent(studentId);
    if (!student) {
        errorMsg = "Student not found.";
        return false;
    }
    if (student->status == StudentStatus::CANCELLED) {
        errorMsg = "Registration is already cancelled.";
        return false;
    }

    if (student->status == StudentStatus::ALLOCATED) {
        std::string vacErr;
        vacateStudent(studentId, vacErr);
    } else if (student->status == StudentStatus::WAITING) {
        priorityQueue_.removeIf([&studentId](const StudentPriorityRef& r) { return r.studentId == studentId; });
        normalQueue_.remove(studentId);
    }

    student->status = StudentStatus::CANCELLED;
    return true;
}

bool HostelSystem::undo(std::string& message) {
    if (undoStack_.empty()) {
        message = "Nothing to undo.";
        return false;
    }
    auto cmd = undoStack_.top();
    undoStack_.pop();
    bool res = cmd->undo(*this);
    if (res) {
        message = "Undid: " + cmd->getDescription();
        redoStack_.push(cmd);
    } else {
        message = "Failed to undo: " + cmd->getDescription();
    }
    return res;
}

bool HostelSystem::redo(std::string& message) {
    if (redoStack_.empty()) {
        message = "Nothing to redo.";
        return false;
    }
    auto cmd = redoStack_.top();
    redoStack_.pop();
    bool res = cmd->redo(*this);
    if (res) {
        message = "Redid: " + cmd->getDescription();
        undoStack_.push(cmd);
    } else {
        message = "Failed to redo: " + cmd->getDescription();
    }
    return res;
}

// Low-level atomic primitives for Undo/Redo
bool HostelSystem::rawAllocate(const std::string& studentId, const std::string& roomId, const std::string& date) {
    Student* student = getStudent(studentId);
    Room* room = getRoom(roomId);
    if (!student || !room) return false;

    // Remove from queues if present
    priorityQueue_.removeIf([&studentId](const StudentPriorityRef& r) { return r.studentId == studentId; });
    normalQueue_.remove(studentId);

    room->allocateBed();
    student->status = StudentStatus::ALLOCATED;
    student->allocatedRoomId = roomId;
    student->allocatedDate = date.empty() ? getCurrentDate() : date;

    allocatedList_.push_back(AllocationRecord(student->id, room->id, student->name, student->allocatedDate, room->occupiedCount));
    return true;
}

bool HostelSystem::rawDeallocate(const std::string& studentId, bool returnToWaitingQueue, bool wasPriorityQueue) {
    Student* student = getStudent(studentId);
    if (!student) return false;

    if (!student->allocatedRoomId.empty()) {
        Room* room = getRoom(student->allocatedRoomId);
        if (room) room->freeBed();
    }

    student->allocatedRoomId = "";
    allocatedList_.removeIf([&studentId](const AllocationRecord& rec) {
        return rec.studentId == studentId;
    });

    if (returnToWaitingQueue) {
        student->status = StudentStatus::WAITING;
        if (wasPriorityQueue) {
            priorityQueue_.push({student->id, student->priorityScore, ++registrationCounter_});
        } else {
            normalQueue_.enqueue(student->id);
        }
    } else {
        student->status = StudentStatus::VACATED;
    }
    return true;
}

bool HostelSystem::rawTransfer(const std::string& studentId, const std::string& newRoomId) {
    Student* student = getStudent(studentId);
    Room* newRoom = getRoom(newRoomId);
    if (!student || !newRoom) return false;

    Room* oldRoom = getRoom(student->allocatedRoomId);
    if (oldRoom) oldRoom->freeBed();

    newRoom->allocateBed();
    student->allocatedRoomId = newRoomId;

    auto* node = allocatedList_.findIf([&studentId](const AllocationRecord& rec) {
        return rec.studentId == studentId;
    });
    if (node) {
        node->data.roomId = newRoomId;
    }
    return true;
}

bool HostelSystem::rawCancel(const std::string& studentId) {
    Student* student = getStudent(studentId);
    if (!student) return false;
    priorityQueue_.removeIf([&studentId](const StudentPriorityRef& r) { return r.studentId == studentId; });
    normalQueue_.remove(studentId);
    student->status = StudentStatus::CANCELLED;
    return true;
}

bool HostelSystem::rawRestoreRegistration(const std::string& studentId, bool wasPriorityQueue) {
    Student* student = getStudent(studentId);
    if (!student) return false;
    student->status = StudentStatus::WAITING;
    if (wasPriorityQueue) {
        priorityQueue_.push({student->id, student->priorityScore, ++registrationCounter_});
    } else {
        normalQueue_.enqueue(student->id);
    }
    return true;
}

dsa::Vector<Student> HostelSystem::getStudentReport(SortField field, bool ascending, SortAlgorithmType algo,
                                                    dsa::SortMetrics& metrics, double& elapsedMicroseconds) const {
    dsa::Vector<Student> reportList;
    reportList.reserve(students_.size());
    for (size_t i = 0; i < students_.size(); ++i) {
        if (students_[i]) {
            reportList.push_back(*students_[i]);
        }
    }

    auto comp = [field, ascending](const Student& a, const Student& b) -> bool {
        bool less = false;
        switch (field) {
            case SortField::NAME:
                less = (a.name < b.name);
                break;
            case SortField::ID:
                less = (a.id < b.id);
                break;
            case SortField::YEAR:
                less = (a.year < b.year);
                break;
            case SortField::PRIORITY:
                less = (a.priorityScore < b.priorityScore);
                break;
            case SortField::CGPA:
                less = (a.cgpa < b.cgpa);
                break;
        }
        return ascending ? less : (!less && (a.id != b.id));
    };

    auto start = std::chrono::high_resolution_clock::now();

    if (algo == SortAlgorithmType::QUICK_SORT) {
        dsa::QuickSort<Student, decltype(comp)>::sort(reportList.data(), reportList.size(), comp, &metrics);
    } else if (algo == SortAlgorithmType::MERGE_SORT) {
        dsa::MergeSort<Student, decltype(comp)>::sort(reportList.data(), reportList.size(), comp, &metrics);
    } else if (algo == SortAlgorithmType::HEAP_SORT) {
        dsa::HeapSort<Student, decltype(comp)>::sort(reportList.data(), reportList.size(), comp, &metrics);
    }

    auto end = std::chrono::high_resolution_clock::now();
    elapsedMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();

    return reportList;
}

dsa::Vector<Room> HostelSystem::getRoomReport(bool onlyAvailable) const {
    dsa::Vector<Room> list;
    for (size_t i = 0; i < rooms_.size(); ++i) {
        if (rooms_[i]) {
            if (!onlyAvailable || rooms_[i]->isAvailable()) {
                list.push_back(*rooms_[i]);
            }
        }
    }
    return list;
}

} // namespace hostel
