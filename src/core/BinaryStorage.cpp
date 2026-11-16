#include "core/BinaryStorage.hpp"
#include <iostream>

namespace hostel {

void BinaryStorage::writeString(std::ofstream& ofs, const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        ofs.write(str.data(), len);
    }
}

std::string BinaryStorage::readString(std::ifstream& ifs) {
    uint32_t len = 0;
    ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (len == 0 || !ifs) return "";
    std::string str(len, '\0');
    ifs.read(&str[0], len);
    return str;
}

bool BinaryStorage::saveToFile(const HostelSystem& system, const std::string& filepath) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) return false;

    // Header
    uint32_t magic = MAGIC_HEADER;
    uint32_t ver = STORAGE_VERSION;
    ofs.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    ofs.write(reinterpret_cast<const char*>(&ver), sizeof(ver));

    // Save Rooms
    const auto& rooms = system.getAllRooms();
    uint32_t roomCount = static_cast<uint32_t>(rooms.size());
    ofs.write(reinterpret_cast<const char*>(&roomCount), sizeof(roomCount));

    for (size_t i = 0; i < rooms.size(); ++i) {
        const auto& r = *rooms[i];
        writeString(ofs, r.id);
        writeString(ofs, r.block);
        ofs.write(reinterpret_cast<const char*>(&r.floor), sizeof(r.floor));
        ofs.write(reinterpret_cast<const char*>(&r.capacity), sizeof(r.capacity));
        ofs.write(reinterpret_cast<const char*>(&r.occupiedCount), sizeof(r.occupiedCount));
        writeString(ofs, r.roomType);
        ofs.write(reinterpret_cast<const char*>(&r.isAC), sizeof(r.isAC));
        writeString(ofs, r.allowedGender);
        int statusInt = static_cast<int>(r.status);
        ofs.write(reinterpret_cast<const char*>(&statusInt), sizeof(statusInt));
        ofs.write(reinterpret_cast<const char*>(&r.monthlyFee), sizeof(r.monthlyFee));
    }

    // Save Students
    const auto& students = system.getAllStudents();
    uint32_t stuCount = static_cast<uint32_t>(students.size());
    ofs.write(reinterpret_cast<const char*>(&stuCount), sizeof(stuCount));

    for (size_t i = 0; i < students.size(); ++i) {
        const auto& s = *students[i];
        writeString(ofs, s.id);
        writeString(ofs, s.name);
        ofs.write(reinterpret_cast<const char*>(&s.age), sizeof(s.age));
        writeString(ofs, s.gender);
        writeString(ofs, s.course);
        ofs.write(reinterpret_cast<const char*>(&s.year), sizeof(s.year));
        ofs.write(reinterpret_cast<const char*>(&s.cgpa), sizeof(s.cgpa));
        ofs.write(reinterpret_cast<const char*>(&s.distanceKm), sizeof(s.distanceKm));
        ofs.write(reinterpret_cast<const char*>(&s.isSpecialCategory), sizeof(s.isSpecialCategory));
        ofs.write(reinterpret_cast<const char*>(&s.priorityScore), sizeof(s.priorityScore));
        writeString(ofs, s.phone);
        writeString(ofs, s.email);
        writeString(ofs, s.guardian);
        writeString(ofs, s.city);
        writeString(ofs, s.bloodGroup);
        int sInt = static_cast<int>(s.status);
        ofs.write(reinterpret_cast<const char*>(&sInt), sizeof(sInt));
        writeString(ofs, s.allocatedRoomId);
        writeString(ofs, s.allocatedDate);
    }

    return ofs.good();
}

bool BinaryStorage::loadFromFile(HostelSystem& system, const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) return false;

    uint32_t magic = 0, ver = 0;
    ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    ifs.read(reinterpret_cast<char*>(&ver), sizeof(ver));

    if (magic != MAGIC_HEADER || ver != STORAGE_VERSION) {
        return false;
    }

    // Read Rooms
    uint32_t roomCount = 0;
    ifs.read(reinterpret_cast<char*>(&roomCount), sizeof(roomCount));
    for (uint32_t i = 0; i < roomCount; ++i) {
        Room r;
        r.id = readString(ifs);
        r.block = readString(ifs);
        ifs.read(reinterpret_cast<char*>(&r.floor), sizeof(r.floor));
        ifs.read(reinterpret_cast<char*>(&r.capacity), sizeof(r.capacity));
        ifs.read(reinterpret_cast<char*>(&r.occupiedCount), sizeof(r.occupiedCount));
        r.roomType = readString(ifs);
        ifs.read(reinterpret_cast<char*>(&r.isAC), sizeof(r.isAC));
        r.allowedGender = readString(ifs);
        int statusInt = 0;
        ifs.read(reinterpret_cast<char*>(&statusInt), sizeof(statusInt));
        r.status = static_cast<RoomStatus>(statusInt);
        ifs.read(reinterpret_cast<char*>(&r.monthlyFee), sizeof(r.monthlyFee));

        std::string err;
        if (!system.getRoom(r.id)) {
            system.addRoom(r, err);
        }
    }

    // Read Students
    uint32_t stuCount = 0;
    ifs.read(reinterpret_cast<char*>(&stuCount), sizeof(stuCount));
    for (uint32_t i = 0; i < stuCount; ++i) {
        Student s;
        s.id = readString(ifs);
        s.name = readString(ifs);
        ifs.read(reinterpret_cast<char*>(&s.age), sizeof(s.age));
        s.gender = readString(ifs);
        s.course = readString(ifs);
        ifs.read(reinterpret_cast<char*>(&s.year), sizeof(s.year));
        ifs.read(reinterpret_cast<char*>(&s.cgpa), sizeof(s.cgpa));
        ifs.read(reinterpret_cast<char*>(&s.distanceKm), sizeof(s.distanceKm));
        ifs.read(reinterpret_cast<char*>(&s.isSpecialCategory), sizeof(s.isSpecialCategory));
        ifs.read(reinterpret_cast<char*>(&s.priorityScore), sizeof(s.priorityScore));
        s.phone = readString(ifs);
        s.email = readString(ifs);
        s.guardian = readString(ifs);
        s.city = readString(ifs);
        s.bloodGroup = readString(ifs);
        int sInt = 0;
        ifs.read(reinterpret_cast<char*>(&sInt), sizeof(sInt));
        s.status = static_cast<StudentStatus>(sInt);
        s.allocatedRoomId = readString(ifs);
        s.allocatedDate = readString(ifs);

        std::string err;
        if (!system.getStudent(s.id)) {
            if (s.status == StudentStatus::ALLOCATED) {
                // Restore allocated student directly
                system.rawAllocate(s.id, s.allocatedRoomId, s.allocatedDate);
            } else if (s.status == StudentStatus::WAITING) {
                system.registerStudent(s, err);
            }
        }
    }

    return ifs.good();
}

} // namespace hostel
