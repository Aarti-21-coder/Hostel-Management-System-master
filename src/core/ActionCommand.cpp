#include "core/ActionCommand.hpp"
#include "core/HostelSystem.hpp"

namespace hostel {

// AllocateCommand Implementation
bool AllocateCommand::undo(HostelSystem& system) {
    return system.rawDeallocate(studentId_, true, wasPriorityQueue_);
}

bool AllocateCommand::redo(HostelSystem& system) {
    return system.rawAllocate(studentId_, roomId_, allocatedDate_);
}

// VacateCommand Implementation
bool VacateCommand::undo(HostelSystem& system) {
    // If vacating this student had auto-allocated someone else, we must first reverse that auto-allocation
    if (!autoAllocatedStudentId_.empty()) {
        Student* autoStudent = system.getStudent(autoAllocatedStudentId_);
        bool wasPri = (autoStudent != nullptr && autoStudent->priorityScore >= 50.0f);
        system.rawDeallocate(autoAllocatedStudentId_, true, wasPri);
    }
    // Now restore the original student back into the room
    return system.rawAllocate(studentId_, roomId_, "Restored");
}

bool VacateCommand::redo(HostelSystem& system) {
    std::string err;
    std::string newAutoAllocId;
    bool success = system.vacateStudent(studentId_, err, &newAutoAllocId);
    if (success) {
        autoAllocatedStudentId_ = newAutoAllocId;
    }
    return success;
}

// TransferCommand Implementation
bool TransferCommand::undo(HostelSystem& system) {
    return system.rawTransfer(studentId_, fromRoomId_);
}

bool TransferCommand::redo(HostelSystem& system) {
    return system.rawTransfer(studentId_, toRoomId_);
}

// RegisterCommand Implementation
bool RegisterCommand::undo(HostelSystem& system) {
    return system.rawCancel(studentId_);
}

bool RegisterCommand::redo(HostelSystem& system) {
    return system.rawRestoreRegistration(studentId_, wasPriority_);
}

} // namespace hostel
