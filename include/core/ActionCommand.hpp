#pragma once

#include <string>
#include <memory>
#include "models/Student.hpp"

namespace hostel {

class HostelSystem;

enum class ActionType {
    ALLOCATE,
    VACATE,
    TRANSFER,
    REGISTER
};

/**
 * @brief Abstract Base Command for the Undo/Redo Stacks.
 */
class ActionCommand {
public:
    virtual ~ActionCommand() = default;
    virtual ActionType getType() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool undo(HostelSystem& system) = 0;
    virtual bool redo(HostelSystem& system) = 0;
};

/**
 * @brief Command for Room Allocation.
 */
class AllocateCommand : public ActionCommand {
private:
    std::string studentId_;
    std::string roomId_;
    std::string allocatedDate_;
    bool wasPriorityQueue_;

public:
    AllocateCommand(std::string sId, std::string rId, std::string date, bool wasPriority)
        : studentId_(std::move(sId)), roomId_(std::move(rId)),
          allocatedDate_(std::move(date)), wasPriorityQueue_(wasPriority) {}

    ActionType getType() const override { return ActionType::ALLOCATE; }
    std::string getDescription() const override {
        return "Allocate student " + studentId_ + " to room " + roomId_;
    }

    bool undo(HostelSystem& system) override;
    bool redo(HostelSystem& system) override;
};

/**
 * @brief Command for Vacating Room (and reversing any auto-allocation).
 */
class VacateCommand : public ActionCommand {
private:
    std::string studentId_;
    std::string roomId_;
    std::string autoAllocatedStudentId_; // populated if vacating triggered auto-allocation

public:
    VacateCommand(std::string sId, std::string rId, std::string autoAllocId = "")
        : studentId_(std::move(sId)), roomId_(std::move(rId)),
          autoAllocatedStudentId_(std::move(autoAllocId)) {}

    void setAutoAllocatedStudent(std::string autoId) {
        autoAllocatedStudentId_ = std::move(autoId);
    }

    ActionType getType() const override { return ActionType::VACATE; }
    std::string getDescription() const override {
        return "Vacate student " + studentId_ + " from room " + roomId_;
    }

    bool undo(HostelSystem& system) override;
    bool redo(HostelSystem& system) override;
};

/**
 * @brief Command for Room Transfer.
 */
class TransferCommand : public ActionCommand {
private:
    std::string studentId_;
    std::string fromRoomId_;
    std::string toRoomId_;

public:
    TransferCommand(std::string sId, std::string fromRoom, std::string toRoom)
        : studentId_(std::move(sId)), fromRoomId_(std::move(fromRoom)),
          toRoomId_(std::move(toRoom)) {}

    ActionType getType() const override { return ActionType::TRANSFER; }
    std::string getDescription() const override {
        return "Transfer student " + studentId_ + " from " + fromRoomId_ + " to " + toRoomId_;
    }

    bool undo(HostelSystem& system) override;
    bool redo(HostelSystem& system) override;
};

/**
 * @brief Command for Student Registration.
 */
class RegisterCommand : public ActionCommand {
private:
    std::string studentId_;
    bool wasPriority_;

public:
    RegisterCommand(std::string sId, bool wasPriority)
        : studentId_(std::move(sId)), wasPriority_(wasPriority) {}

    ActionType getType() const override { return ActionType::REGISTER; }
    std::string getDescription() const override {
        return "Register student " + studentId_;
    }

    bool undo(HostelSystem& system) override;
    bool redo(HostelSystem& system) override;
};

} // namespace hostel
