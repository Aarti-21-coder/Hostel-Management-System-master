#pragma once

#include <string>
#include <memory>
#include <chrono>

#include "dsa/Vector.hpp"
#include "dsa/HashTable.hpp"
#include "dsa/BinaryHeap.hpp"
#include "dsa/CircularQueue.hpp"
#include "dsa/Stack.hpp"
#include "dsa/DoublyLinkedList.hpp"
#include "dsa/Sort.hpp"

#include "models/Student.hpp"
#include "models/Room.hpp"
#include "models/Allocation.hpp"
#include "core/ActionCommand.hpp"

namespace hostel {

enum class SortField {
    NAME,
    ID,
    YEAR,
    PRIORITY,
    CGPA
};

enum class SortAlgorithmType {
    QUICK_SORT,
    MERGE_SORT,
    HEAP_SORT
};

/**
 * @brief Main Hostel Management System Controller.
 * 
 * Implements the Core DFD specification:
 *  - 1.0 Main Process / Controller
 *  - 2.0 Student Registration (Vector + Hash Index + Priority/Normal Waiting Lists)
 *  - 3.0 Room Allocation & Availability Manager (Doubly Linked List Allocated DB)
 *  - 4.0 View Reports with Custom Quick/Merge/HeapSort
 *  - Extra Flows: Vacate with Auto-Reallocation, Transfer, Cancel, Undo/Redo Stacks
 */
class HostelSystem {
private:
    // Core Data Stores (Shared pointers ensure 100% pointer stability on vector resizing)
    dsa::Vector<std::shared_ptr<Student>> students_;      // Master student records
    dsa::HashTable<std::string, Student*> studentMap_;    // O(1) Student ID Index
    dsa::Vector<std::shared_ptr<Room>> rooms_;            // Room records
    dsa::HashTable<std::string, Room*> roomMap_;          // O(1) Room ID Index

    // Waiting Lists
    dsa::MaxHeap<StudentPriorityRef> priorityQueue_;      // Binary Max-Heap for High Priority
    dsa::CircularQueue<std::string> normalQueue_;         // Circular Queue for Normal FIFO
    int registrationCounter_ = 0;                         // Deterministic tie-breaker

    // Allocated Students DB
    dsa::DoublyLinkedList<AllocationRecord> allocatedList_; // Doubly Linked List

    // Undo / Redo Stacks (Command Pattern)
    dsa::Stack<std::shared_ptr<ActionCommand>> undoStack_;
    dsa::Stack<std::shared_ptr<ActionCommand>> redoStack_;

    // Private helper methods
    Room* findAvailableRoom(const std::string& gender, const std::string& preferredType = "");
    std::string getCurrentDate();

public:
    explicit HostelSystem(size_t normalQueueCapacity = 1000);
    ~HostelSystem() = default;

    // Room Inventory Management
    bool addRoom(const Room& room, std::string& errorMsg);
    Room* getRoom(const std::string& roomId);
    const Room* getRoom(const std::string& roomId) const;
    const dsa::Vector<std::shared_ptr<Room>>& getAllRooms() const { return rooms_; }

    // DFD 2.0: Student Registration
    bool registerStudent(Student student, std::string& errorMsg);
    Student* getStudent(const std::string& studentId);
    const Student* getStudent(const std::string& studentId) const;
    const dsa::Vector<std::shared_ptr<Student>>& getAllStudents() const { return students_; }

    // Waiting Lists Inspection
    size_t getPriorityQueueSize() const { return priorityQueue_.size(); }
    size_t getNormalQueueSize() const { return normalQueue_.size(); }
    dsa::Vector<StudentPriorityRef> getPriorityQueueSnapshot() const;
    dsa::Vector<std::string> getNormalQueueSnapshot() const;

    // DFD 3.0: Room Allocation & Availability
    bool allocateNext(std::string& allocatedStudentId, std::string& allocatedRoomId, std::string& errorMsg);
    bool allocateStudentToRoom(const std::string& studentId, const std::string& roomId, std::string& errorMsg);

    // Extra Flow: Vacate with Immediate Auto-Reallocation
    bool vacateStudent(const std::string& studentId, std::string& errorMsg, std::string* autoAllocatedStudentId = nullptr);

    // Extra Flow: Transfer Room
    bool transferStudent(const std::string& studentId, const std::string& newRoomId, std::string& errorMsg);

    // Extra Flow: Cancel Registration
    bool cancelRegistration(const std::string& studentId, std::string& errorMsg);

    // Undo / Redo
    bool undo(std::string& message);
    bool redo(std::string& message);
    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    // Allocated Students DB Inspection
    const dsa::DoublyLinkedList<AllocationRecord>& getAllocatedList() const { return allocatedList_; }

    // DFD 4.0: View Reports with Custom Sorts
    dsa::Vector<Student> getStudentReport(SortField field, bool ascending, SortAlgorithmType algo,
                                         dsa::SortMetrics& metrics, double& elapsedMicroseconds) const;
    dsa::Vector<Room> getRoomReport(bool onlyAvailable) const;

    // Direct internals access for Undo/Redo commands
    bool rawAllocate(const std::string& studentId, const std::string& roomId, const std::string& date);
    bool rawDeallocate(const std::string& studentId, bool returnToWaitingQueue, bool wasPriorityQueue);
    bool rawTransfer(const std::string& studentId, const std::string& newRoomId);
    bool rawCancel(const std::string& studentId);
    bool rawRestoreRegistration(const std::string& studentId, bool wasPriorityQueue);

    // Seed sample default rooms (Blocks A-E)
    void seedDefaultRooms();
};

} // namespace hostel
