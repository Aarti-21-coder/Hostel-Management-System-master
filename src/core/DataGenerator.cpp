#include "core/DataGenerator.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace hostel {

static const char* FIRST_NAMES[] = {
    "Aarav", "Vivaan", "Aditya", "Vihaan", "Arjun", "Sai", "Reyansh", "Ayaan", "Krishna", "Ishaan",
    "Shaurya", "Atharva", "Advik", "Pranav", "Advaith", "Aaryan", "Dhruv", "Kabir", "Ritvik", "Darsh",
    "Ananya", "Diya", "Saanvi", "Aadhya", "Pari", "Chiara", "Myra", "Riya", "Anvi", "Aarohi",
    "Isha", "Navya", "Avani", "Kavya", "Tanvi", "Siya", "Vanya", "Prisha", "Sara", "Meera"
};
static constexpr size_t NUM_FIRST = sizeof(FIRST_NAMES) / sizeof(FIRST_NAMES[0]);

static const char* LAST_NAMES[] = {
    "Sharma", "Verma", "Patel", "Gupta", "Mehta", "Iyer", "Rao", "Nair", "Reddy", "Singh",
    "Kumar", "Das", "Joshi", "Bose", "Chopra", "Malhotra", "Kapoor", "Bhatia", "Saxena", "Pandey"
};
static constexpr size_t NUM_LAST = sizeof(LAST_NAMES) / sizeof(LAST_NAMES[0]);

static const char* COURSES[] = {
    "B.Tech CSE", "B.Tech ECE", "B.Tech ME", "B.Tech CE", "B.Tech EE",
    "B.Tech AI&DS", "B.Tech IT", "MCA", "M.Tech CSE", "MBA Tech"
};
static constexpr size_t NUM_COURSES = sizeof(COURSES) / sizeof(COURSES[0]);

static const char* CITIES[] = {
    "Mumbai", "Delhi", "Bengaluru", "Hyderabad", "Chennai", "Kolkata", "Pune", "Ahmedabad",
    "Jaipur", "Lucknow", "Chandigarh", "Bhopal", "Indore", "Patna", "Kochi", "Nagpur"
};
static constexpr size_t NUM_CITIES = sizeof(CITIES) / sizeof(CITIES[0]);

static const char* BLOOD_GROUPS[] = {
    "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"
};
static constexpr size_t NUM_BLOOD = sizeof(BLOOD_GROUPS) / sizeof(BLOOD_GROUPS[0]);

size_t DataGenerator::generateRooms(HostelSystem& system, size_t targetCount) {
    size_t added = 0;
    std::string blocks[] = {"A", "B", "C", "D", "E"};
    int floor = 1;

    while (added < targetCount) {
        for (const auto& blk : blocks) {
            std::string blockName = "Block " + blk;
            std::string gender = (blk == "A" || blk == "B" || blk == "C") ? "Male" : "Female";

            for (int r = 1; r <= 20; ++r) {
                if (added >= targetCount) break;

                std::ostringstream oss;
                oss << blk << "-" << floor << (r < 10 ? "0" : "") << r;
                std::string rId = oss.str();

                if (!system.getRoom(rId)) {
                    int cap = (r % 3 == 0) ? 3 : ((r % 2 == 0) ? 2 : 1);
                    std::string rType = (cap == 1) ? "Single" : ((cap == 2) ? "Double" : "Triple");
                    bool ac = (floor % 2 == 0);
                    float fee = (cap == 1 ? 9500.0f : (cap == 2 ? 6500.0f : 4800.0f)) + (ac ? 2000.0f : 0.0f);

                    Room rm(rId, blockName, floor, cap, rType, ac, gender, fee);
                    std::string err;
                    if (system.addRoom(rm, err)) {
                        ++added;
                    }
                }
            }
        }
        ++floor;
    }
    return added;
}

size_t DataGenerator::generateStudents(HostelSystem& system, size_t count) {
    size_t startId = system.getAllStudents().size() + 1;
    size_t registered = 0;

    std::mt19937 gen(42 + static_cast<unsigned int>(startId));
    std::uniform_real_distribution<float> cgpaDist(5.5f, 9.9f);
    std::uniform_real_distribution<float> distDist(20.0f, 1500.0f);
    std::uniform_int_distribution<int> yearDist(1, 4);
    std::uniform_int_distribution<int> specialDist(1, 10); // 10% special category

    for (size_t i = 0; i < count; ++i) {
        size_t curIdx = startId + i;
        std::string sId = "STU" + std::to_string(100000 + curIdx);

        std::string firstName = FIRST_NAMES[curIdx % NUM_FIRST];
        std::string lastName = LAST_NAMES[(curIdx / NUM_FIRST) % NUM_LAST];
        std::string fullName = firstName + " " + lastName;
        std::string gender = (curIdx % NUM_FIRST < 20) ? "Male" : "Female";
        std::string course = COURSES[curIdx % NUM_COURSES];
        int year = yearDist(gen);
        int age = 17 + year + (curIdx % 2);
        float cgpa = cgpaDist(gen);
        float dist = distDist(gen);
        bool isSpecial = (specialDist(gen) == 1);

        std::string phone = "+91" + std::to_string(9800000000ULL + (curIdx * 37) % 199999999ULL);
        std::string email = "stu" + std::to_string(curIdx) + "@hostel.edu";
        std::string city = CITIES[curIdx % NUM_CITIES];
        std::string guardian = "Mr. " + lastName;
        std::string blood = BLOOD_GROUPS[curIdx % NUM_BLOOD];

        Student s(sId, fullName, age, gender, course, year, cgpa, dist, isSpecial,
                  phone, email, guardian, city, blood);

        std::string err;
        if (system.registerStudent(s, err)) {
            ++registered;
        }
    }

    return registered;
}

} // namespace hostel
