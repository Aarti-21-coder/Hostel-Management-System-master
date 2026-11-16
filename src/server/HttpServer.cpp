#define NOMINMAX
#include "server/HttpServer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#include "extern/httplib.h"
#include "extern/json.hpp"
#include "core/DataGenerator.hpp"
#include "core/BinaryStorage.hpp"
#include "dsa/AVLTree.hpp"

using json = nlohmann::json;

namespace hostel {

HttpServer::HttpServer(std::shared_ptr<HostelSystem> sys,
                       std::shared_ptr<AuthManager> auth,
                       int port,
                       std::string frontendPath)
    : system_(sys), auth_(auth), port_(port), frontendPath_(std::move(frontendPath)) {
    server_ = std::make_unique<httplib::Server>();
    initCampusGraph();
    syncSearchTrie();
    setupRoutes();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::initCampusGraph() {
    int bA = campusGraph_.addNode("Block A", "Hostel Block");
    int bB = campusGraph_.addNode("Block B", "Hostel Block");
    int bC = campusGraph_.addNode("Block C", "Hostel Block");
    int bD = campusGraph_.addNode("Block D", "Hostel Block");
    int bE = campusGraph_.addNode("Block E", "Hostel Block");
    int mess = campusGraph_.addNode("Dining Hall / Mess", "Mess");
    int gate = campusGraph_.addNode("Main Campus Gate", "Gate");
    int clinic = campusGraph_.addNode("Health & Wellness Center", "Medical");
    int lib = campusGraph_.addNode("Central Library", "Academic");
    int sports = campusGraph_.addNode("Sports Complex", "Sports");

    campusGraph_.addEdge(bA, bB, 80.0f, "Walkway A-B");
    campusGraph_.addEdge(bB, bC, 90.0f, "Walkway B-C");
    campusGraph_.addEdge(bC, bD, 75.0f, "Central Lawn Pathway");
    campusGraph_.addEdge(bD, bE, 85.0f, "Walkway D-E");

    campusGraph_.addEdge(bA, mess, 180.0f, "Avenue Path");
    campusGraph_.addEdge(bB, mess, 120.0f, "North Courtyard");
    campusGraph_.addEdge(bC, mess, 110.0f, "Main Promenade");
    campusGraph_.addEdge(bD, mess, 130.0f, "South Walkway");
    campusGraph_.addEdge(bE, mess, 190.0f, "Perimeter Road");

    campusGraph_.addEdge(mess, lib, 140.0f, "Academic Corridor");
    campusGraph_.addEdge(lib, gate, 160.0f, "Main Boulevard");
    campusGraph_.addEdge(mess, clinic, 100.0f, "Medical Crossway");
    campusGraph_.addEdge(clinic, gate, 120.0f, "Emergency Exit Gate");
    campusGraph_.addEdge(mess, sports, 150.0f, "Track Path");
    campusGraph_.addEdge(sports, gate, 200.0f, "Outer Ring Road");
}

void HttpServer::syncSearchTrie() {
    const auto& students = system_->getAllStudents();
    for (size_t i = 0; i < students.size(); ++i) {
        if (students[i]) {
            searchTrie_.insert(students[i]->name, students[i]->id);
            searchTrie_.insert(students[i]->id, students[i]->id);
        }
    }
}

void HttpServer::setupRoutes() {
    // Enable CORS on all responses
    server_->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    server_->Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
    });

    // 1. Root & Static Web UI Serving
    server_->set_mount_point("/", frontendPath_);

    // 2. Auth APIs
    server_->Post("/api/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        try {
            auto body = json::parse(req.body);
            std::string username = body.value("username", "");
            std::string password = body.value("password", "");

            std::string token;
            UserRole role;
            std::string err;
            if (auth_->login(username, password, token, role, err)) {
                json resp = {
                    {"success", true},
                    {"token", token},
                    {"username", username},
                    {"role", roleToString(role)},
                    {"message", "Login successful"}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                json resp = {
                    {"success", false},
                    {"error", err}
                };
                res.status = 401;
                res.set_content(resp.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    server_->Post("/api/auth/register", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        try {
            auto body = json::parse(req.body);
            std::string username = body.value("username", "");
            std::string password = body.value("password", "");
            std::string roleStr = body.value("role", "Student");
            std::string email = body.value("email", "");
            std::string fullName = body.value("fullName", "");

            std::string err;
            if (auth_->registerUser(username, password, stringToRole(roleStr), email, fullName, err)) {
                res.set_content(json{{"success", true}, {"message", "Registration successful. You can now login."}}.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 3. Stats & Overview (DFD 1.0)
    server_->Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
        const auto& students = system_->getAllStudents();
        const auto& rooms = system_->getAllRooms();

        size_t waitingCount = 0;
        size_t allocatedCount = 0;
        size_t vacatedCount = 0;
        size_t cancelledCount = 0;

        for (size_t i = 0; i < students.size(); ++i) {
            if (students[i]) {
                if (students[i]->status == StudentStatus::WAITING) ++waitingCount;
                else if (students[i]->status == StudentStatus::ALLOCATED) ++allocatedCount;
                else if (students[i]->status == StudentStatus::VACATED) ++vacatedCount;
                else if (students[i]->status == StudentStatus::CANCELLED) ++cancelledCount;
            }
        }

        size_t totalBeds = 0;
        size_t occupiedBeds = 0;
        for (size_t i = 0; i < rooms.size(); ++i) {
            if (rooms[i]) {
                totalBeds += rooms[i]->capacity;
                occupiedBeds += rooms[i]->occupiedCount;
            }
        }

        float occupancyPercent = (totalBeds > 0) ? (static_cast<float>(occupiedBeds) * 100.0f / totalBeds) : 0.0f;

        json resp = {
            {"totalStudents", students.size()},
            {"waitingCount", waitingCount},
            {"allocatedCount", allocatedCount},
            {"vacatedCount", vacatedCount},
            {"cancelledCount", cancelledCount},
            {"priorityQueueCount", system_->getPriorityQueueSize()},
            {"normalQueueCount", system_->getNormalQueueSize()},
            {"totalRooms", rooms.size()},
            {"totalBeds", totalBeds},
            {"occupiedBeds", occupiedBeds},
            {"availableBeds", (totalBeds > occupiedBeds) ? (totalBeds - occupiedBeds) : 0},
            {"occupancyPercent", occupancyPercent},
            {"canUndo", system_->canUndo()},
            {"canRedo", system_->canRedo()}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 4. Student Registration (DFD 2.0)
    server_->Post("/api/students/register", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        try {
            auto body = json::parse(req.body);
            std::string id = body.value("id", "");
            std::string name = body.value("name", "");
            int age = body.value("age", 18);
            std::string gender = body.value("gender", "Male");
            std::string course = body.value("course", "CSE");
            int year = body.value("year", 1);
            float cgpa = body.value("cgpa", 7.0f);
            float dist = body.value("distanceKm", 50.0f);
            bool isSpec = body.value("isSpecialCategory", false);
            std::string phone = body.value("phone", "");
            std::string email = body.value("email", "");
            std::string city = body.value("city", "");

            Student st(id, name, age, gender, course, year, cgpa, dist, isSpec, phone, email, "", city, "");
            std::string err;
            if (system_->registerStudent(st, err)) {
                searchTrie_.insert(name, id);
                searchTrie_.insert(id, id);

                Student* reg = system_->getStudent(id);
                bool isPri = (reg->priorityScore >= 50.0f);

                json resp = {
                    {"success", true},
                    {"id", reg->id},
                    {"name", reg->name},
                    {"priorityScore", reg->priorityScore},
                    {"queueType", isPri ? "Priority Queue (Binary Max-Heap)" : "Normal Queue (Circular FIFO)"},
                    {"message", "Student successfully registered and queued for allocation."}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 5. Waiting Lists (Priority & Normal Tabs)
    server_->Get("/api/waiting-list", [this](const httplib::Request&, httplib::Response& res) {
        json prioList = json::array();
        auto prioSnap = system_->getPriorityQueueSnapshot();
        for (size_t i = 0; i < prioSnap.size(); ++i) {
            const Student* s = system_->getStudent(prioSnap[i].studentId);
            if (s) {
                prioList.push_back({
                    {"id", s->id},
                    {"name", s->name},
                    {"course", s->course},
                    {"year", s->year},
                    {"cgpa", s->cgpa},
                    {"priorityScore", s->priorityScore},
                    {"gender", s->gender},
                    {"city", s->city}
                });
            }
        }

        json normList = json::array();
        auto normSnap = system_->getNormalQueueSnapshot();
        for (size_t i = 0; i < normSnap.size(); ++i) {
            const Student* s = system_->getStudent(normSnap[i]);
            if (s) {
                normList.push_back({
                    {"id", s->id},
                    {"name", s->name},
                    {"course", s->course},
                    {"year", s->year},
                    {"cgpa", s->cgpa},
                    {"priorityScore", s->priorityScore},
                    {"gender", s->gender},
                    {"city", s->city}
                });
            }
        }

        json resp = {
            {"priorityQueue", prioList},
            {"normalQueue", normList},
            {"priorityCount", prioList.size()},
            {"normalCount", normList.size()}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 6. Room Allocation (DFD 3.0)
    server_->Post("/api/rooms/allocate-next", [this](const httplib::Request&, httplib::Response& res) {
        std::string sId, rId, err;
        if (system_->allocateNext(sId, rId, err)) {
            Student* st = system_->getStudent(sId);
            Room* rm = system_->getRoom(rId);
            json resp = {
                {"success", true},
                {"studentId", sId},
                {"studentName", st ? st->name : ""},
                {"roomId", rId},
                {"roomType", rm ? rm->roomType : ""},
                {"block", rm ? rm->block : ""},
                {"floor", rm ? rm->floor : 1},
                {"message", "Allocated " + (st ? st->name : sId) + " to Room " + rId}
            };
            res.set_content(resp.dump(), "application/json");
        } else {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
        }
    });

    server_->Post("/api/rooms/allocate-manual", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string sId = body.value("studentId", "");
            std::string rId = body.value("roomId", "");
            std::string err;
            if (system_->allocateStudentToRoom(sId, rId, err)) {
                res.set_content(json{{"success", true}, {"message", "Student " + sId + " allocated to " + rId}}.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 7. Vacate Room (Triggers Immediate Auto-Reallocation)
    server_->Post("/api/rooms/vacate", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string sId = body.value("studentId", "");
            std::string err, autoAllocId;
            if (system_->vacateStudent(sId, err, &autoAllocId)) {
                json resp = {
                    {"success", true},
                    {"vacatedStudentId", sId},
                    {"autoAllocatedStudentId", autoAllocId},
                    {"message", "Student " + sId + " vacated." +
                                (!autoAllocId.empty() ? " Room was automatically auto-allocated to waiting student " + autoAllocId + "!" : "")}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 8. Transfer Room
    server_->Post("/api/rooms/transfer", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string sId = body.value("studentId", "");
            std::string newRoomId = body.value("newRoomId", "");
            std::string err;
            if (system_->transferStudent(sId, newRoomId, err)) {
                res.set_content(json{{"success", true}, {"message", "Student transferred to " + newRoomId}}.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 9. Cancel Registration
    server_->Post("/api/students/cancel", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string sId = body.value("studentId", "");
            std::string err;
            if (system_->cancelRegistration(sId, err)) {
                res.set_content(json{{"success", true}, {"message", "Student " + sId + " registration cancelled."}}.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"success", false}, {"error", err}}.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    // 10. Allocated Students DB (Doubly Linked List Store)
    server_->Get("/api/allocated", [this](const httplib::Request&, httplib::Response& res) {
        json list = json::array();
        system_->getAllocatedList().forEach([&list](const AllocationRecord& rec) {
            list.push_back({
                {"studentId", rec.studentId},
                {"studentName", rec.studentName},
                {"roomId", rec.roomId},
                {"allocatedDate", rec.allocatedDate},
                {"bedIndex", rec.bedIndex}
            });
        });
        res.set_content(json{{"allocatedStudents", list}, {"count", list.size()}}.dump(), "application/json");
    });

    // 11. Master Student Status DB (with Server-side Pagination & Filter)
    server_->Get("/api/students", [this](const httplib::Request& req, httplib::Response& res) {
        int page = 1;
        int limit = 50;
        if (req.has_param("page")) page = std::max(1, std::stoi(req.get_param_value("page")));
        if (req.has_param("limit")) limit = std::max(5, std::min(500, std::stoi(req.get_param_value("limit"))));

        std::string statusFilter = req.has_param("status") ? req.get_param_value("status") : "ALL";
        std::string search = req.has_param("search") ? req.get_param_value("search") : "";

        const auto& all = system_->getAllStudents();
        json filtered = json::array();

        for (size_t i = 0; i < all.size(); ++i) {
            if (!all[i]) continue;
            const auto& s = *all[i];

            if (statusFilter != "ALL" && statusToString(s.status) != statusFilter) {
                continue;
            }

            if (!search.empty()) {
                if (s.name.find(search) == std::string::npos &&
                    s.id.find(search) == std::string::npos &&
                    s.course.find(search) == std::string::npos) {
                    continue;
                }
            }

            filtered.push_back({
                {"id", s.id},
                {"name", s.name},
                {"age", s.age},
                {"gender", s.gender},
                {"course", s.course},
                {"year", s.year},
                {"cgpa", s.cgpa},
                {"priorityScore", s.priorityScore},
                {"phone", s.phone},
                {"email", s.email},
                {"city", s.city},
                {"status", statusToString(s.status)},
                {"allocatedRoomId", s.allocatedRoomId},
                {"allocatedDate", s.allocatedDate}
            });
        }

        size_t total = filtered.size();
        size_t start = (page - 1) * limit;
        json pageItems = json::array();
        for (size_t i = start; i < start + limit && i < total; ++i) {
            pageItems.push_back(filtered[i]);
        }

        json resp = {
            {"page", page},
            {"limit", limit},
            {"total", total},
            {"totalPages", (total + limit - 1) / limit},
            {"students", pageItems}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 12. Interactive Room Grid per Block & Floor
    server_->Get("/api/rooms", [this](const httplib::Request& req, httplib::Response& res) {
        std::string blockFilter = req.has_param("block") ? req.get_param_value("block") : "";
        int floorFilter = req.has_param("floor") ? std::stoi(req.get_param_value("floor")) : 0;

        const auto& all = system_->getAllRooms();
        json roomsList = json::array();

        for (size_t i = 0; i < all.size(); ++i) {
            if (!all[i]) continue;
            const auto& r = *all[i];

            if (!blockFilter.empty() && r.block.find(blockFilter) == std::string::npos) {
                continue;
            }
            if (floorFilter > 0 && r.floor != floorFilter) {
                continue;
            }

            roomsList.push_back({
                {"id", r.id},
                {"block", r.block},
                {"floor", r.floor},
                {"capacity", r.capacity},
                {"occupiedCount", r.occupiedCount},
                {"roomType", r.roomType},
                {"isAC", r.isAC},
                {"allowedGender", r.allowedGender},
                {"status", roomStatusToString(r.status)},
                {"monthlyFee", r.monthlyFee}
            });
        }

        res.set_content(json{{"rooms", roomsList}, {"count", roomsList.size()}}.dump(), "application/json");
    });

    // 13. DFD 4.0 Reports with QuickSort, MergeSort, HeapSort
    server_->Get("/api/reports", [this](const httplib::Request& req, httplib::Response& res) {
        std::string fieldStr = req.has_param("field") ? req.get_param_value("field") : "cgpa";
        bool asc = req.has_param("ascending") ? (req.get_param_value("ascending") == "true") : false;
        std::string algoStr = req.has_param("algo") ? req.get_param_value("algo") : "quicksort";

        SortField field = SortField::CGPA;
        if (fieldStr == "name") field = SortField::NAME;
        else if (fieldStr == "id") field = SortField::ID;
        else if (fieldStr == "year") field = SortField::YEAR;
        else if (fieldStr == "priority") field = SortField::PRIORITY;

        SortAlgorithmType algo = SortAlgorithmType::QUICK_SORT;
        if (algoStr == "mergesort") algo = SortAlgorithmType::MERGE_SORT;
        else if (algoStr == "heapsort") algo = SortAlgorithmType::HEAP_SORT;

        dsa::SortMetrics metrics;
        double elapsedUs = 0.0;
        auto sorted = system_->getStudentReport(field, asc, algo, metrics, elapsedUs);

        json list = json::array();
        size_t limit = std::min(sorted.size(), static_cast<size_t>(100)); // preview first 100
        for (size_t i = 0; i < limit; ++i) {
            list.push_back({
                {"id", sorted[i].id},
                {"name", sorted[i].name},
                {"course", sorted[i].course},
                {"year", sorted[i].year},
                {"cgpa", sorted[i].cgpa},
                {"priorityScore", sorted[i].priorityScore},
                {"status", statusToString(sorted[i].status)},
                {"allocatedRoomId", sorted[i].allocatedRoomId}
            });
        }

        json resp = {
            {"sortedCount", sorted.size()},
            {"elapsedMicroseconds", elapsedUs},
            {"comparisons", metrics.comparisons},
            {"swaps", metrics.swaps},
            {"algorithm", algoStr},
            {"students", list}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 14. Undo & Redo
    server_->Post("/api/actions/undo", [this](const httplib::Request&, httplib::Response& res) {
        std::string msg;
        if (system_->undo(msg)) {
            res.set_content(json{{"success", true}, {"message", msg}}.dump(), "application/json");
        } else {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", msg}}.dump(), "application/json");
        }
    });

    server_->Post("/api/actions/redo", [this](const httplib::Request&, httplib::Response& res) {
        std::string msg;
        if (system_->redo(msg)) {
            res.set_content(json{{"success", true}, {"message", msg}}.dump(), "application/json");
        } else {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", msg}}.dump(), "application/json");
        }
    });

    // 15. Campus Graph Dijkstra Shortest Path
    server_->Get("/api/campus/route", [this](const httplib::Request& req, httplib::Response& res) {
        std::string from = req.has_param("from") ? req.get_param_value("from") : "Block A";
        std::string to = req.has_param("to") ? req.get_param_value("to") : "Dining Hall / Mess";

        int u = campusGraph_.getNodeIndex(from);
        int v = campusGraph_.getNodeIndex(to);

        if (u == -1 || v == -1) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", "Invalid origin or destination node"}}.dump(), "application/json");
            return;
        }

        auto path = campusGraph_.shortestPath(u, v);
        json pathNames = json::array();
        for (size_t i = 0; i < path.pathNodeNames.size(); ++i) {
            pathNames.push_back(path.pathNodeNames[i]);
        }

        json resp = {
            {"success", path.reachable},
            {"from", from},
            {"to", to},
            {"totalDistanceMeters", path.totalDistance},
            {"path", pathNames}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 16. Search Autocomplete (Trie)
    server_->Get("/api/search/autocomplete", [this](const httplib::Request& req, httplib::Response& res) {
        std::string q = req.has_param("q") ? req.get_param_value("q") : "";
        auto matches = searchTrie_.autocomplete(q, 8);
        json list = json::array();
        for (size_t i = 0; i < matches.size(); ++i) {
            list.push_back({
                {"word", matches[i].word},
                {"payload", matches[i].payload}
            });
        }
        res.set_content(json{{"results", list}}.dump(), "application/json");
    });

    // 17. Big Dataset Generator Buttons
    server_->Post("/api/dataset/generate", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            size_t count = body.value("count", 1000);
            auto start = std::chrono::high_resolution_clock::now();
            size_t gen = DataGenerator::generateStudents(*system_, count);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            syncSearchTrie();
            res.set_content(json{
                {"success", true},
                {"generated", gen},
                {"elapsedMs", ms},
                {"totalStudents", system_->getAllStudents().size()}
            }.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });

    server_->Post("/api/dataset/save", [this](const httplib::Request&, httplib::Response& res) {
        bool ok = BinaryStorage::saveToFile(*system_, "data/hostel_state.bin");
        res.set_content(json{{"success", ok}, {"message", ok ? "State saved to data/hostel_state.bin" : "Save failed"}}.dump(), "application/json");
    });

    server_->Post("/api/dataset/load", [this](const httplib::Request&, httplib::Response& res) {
        auto start = std::chrono::high_resolution_clock::now();
        bool ok = BinaryStorage::loadFromFile(*system_, "data/hostel_state.bin");
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        syncSearchTrie();
        res.set_content(json{
            {"success", ok},
            {"elapsedMs", ms},
            {"totalStudents", system_->getAllStudents().size()},
            {"totalRooms", system_->getAllRooms().size()}
        }.dump(), "application/json");
    });

    // 18. DSA Lab / Live Benchmark Runner
    server_->Post("/api/benchmark/run", [this](const httplib::Request&, httplib::Response& res) {
        // Runs empirical benchmarks across sizes: 100, 500, 1000, 5000
        int sizes[] = {100, 500, 1000, 2500};
        json sortResults = json::array();
        json searchResults = json::array();

        for (int n : sizes) {
            // Benchmark QuickSort vs MergeSort vs HeapSort
            dsa::Vector<int> arrQ(n, 0);
            dsa::Vector<int> arrM(n, 0);
            dsa::Vector<int> arrH(n, 0);
            for (int i = 0; i < n; ++i) {
                int val = (i * 37 + 19) % (n * 3);
                arrQ[i] = val;
                arrM[i] = val;
                arrH[i] = val;
            }

            dsa::SortMetrics mQ, mM, mH;
            auto t0 = std::chrono::high_resolution_clock::now();
            dsa::QuickSort<int>::sort(arrQ.data(), n, std::less<int>(), &mQ);
            auto t1 = std::chrono::high_resolution_clock::now();
            dsa::MergeSort<int>::sort(arrM.data(), n, std::less<int>(), &mM);
            auto t2 = std::chrono::high_resolution_clock::now();
            dsa::HeapSort<int>::sort(arrH.data(), n, std::less<int>(), &mH);
            auto t3 = std::chrono::high_resolution_clock::now();

            double qTime = std::chrono::duration<double, std::micro>(t1 - t0).count();
            double mTime = std::chrono::duration<double, std::micro>(t2 - t1).count();
            double hTime = std::chrono::duration<double, std::micro>(t3 - t2).count();

            sortResults.push_back({
                {"n", n},
                {"quickSortUs", qTime},
                {"mergeSortUs", mTime},
                {"heapSortUs", hTime}
            });

            // Benchmark Linear Search vs HashTable vs AVL Tree
            dsa::HashTable<int, int> ht(n * 2);
            dsa::AVLTree<int, int> avl;
            for (int i = 0; i < n; ++i) {
                ht.insert(arrQ[i], arrQ[i]);
                avl.insert(arrQ[i], arrQ[i]);
            }

            int target = arrQ[n / 2];
            // Linear search
            auto s0 = std::chrono::high_resolution_clock::now();
            volatile int foundLin = -1;
            for (int i = 0; i < n; ++i) {
                if (arrQ[i] == target) { foundLin = arrQ[i]; break; }
            }
            auto s1 = std::chrono::high_resolution_clock::now();
            // HashTable search
            int* foundHt = ht.find(target);
            auto s2 = std::chrono::high_resolution_clock::now();
            // AVL Tree search
            int* foundAvl = avl.find(target);
            auto s3 = std::chrono::high_resolution_clock::now();

            double linUs = std::chrono::duration<double, std::nano>(s1 - s0).count() / 1000.0;
            double htUs = std::chrono::duration<double, std::nano>(s2 - s1).count() / 1000.0;
            double avlUs = std::chrono::duration<double, std::nano>(s3 - s2).count() / 1000.0;

            searchResults.push_back({
                {"n", n},
                {"linearSearchUs", linUs},
                {"hashTableUs", htUs},
                {"avlTreeUs", avlUs}
            });
        }

        json resp = {
            {"sortingBenchmarks", sortResults},
            {"searchBenchmarks", searchResults}
        };
        res.set_content(resp.dump(), "application/json");
    });
}

void HttpServer::start() {
    std::cout << "\n=======================================================\n";
    std::cout << "  HOSTEL MANAGEMENT SYSTEM - WEB SERVER RUNNING\n";
    std::cout << "  URL: http://localhost:" << port_ << "\n";
    std::cout << "  Default Admin Login: admin / Admin@123\n";
    std::cout << "=======================================================\n" << std::endl;
    server_->listen("127.0.0.1", port_);
}

void HttpServer::stop() {
    if (server_) {
        server_->stop();
    }
}

} // namespace hostel
