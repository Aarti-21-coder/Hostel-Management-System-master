#pragma once

#include <string>
#include <chrono>
#include <memory>
#include "dsa/HashTable.hpp"
#include "dsa/Vector.hpp"

namespace hostel {

enum class UserRole {
    ADMIN,
    WARDEN,
    STUDENT,
    GUARD
};

inline std::string roleToString(UserRole role) {
    switch (role) {
        case UserRole::ADMIN:   return "Admin";
        case UserRole::WARDEN:  return "Warden";
        case UserRole::STUDENT: return "Student";
        case UserRole::GUARD:   return "Guard";
        default:                return "Student";
    }
}

inline UserRole stringToRole(const std::string& str) {
    if (str == "Admin" || str == "ADMIN")   return UserRole::ADMIN;
    if (str == "Warden" || str == "WARDEN") return UserRole::WARDEN;
    if (str == "Guard" || str == "GUARD")   return UserRole::GUARD;
    return UserRole::STUDENT;
}

struct User {
    std::string username;
    std::string passwordHash; // $pbkdf2-sha256$i=10000$salt$hash
    std::string salt;
    UserRole role = UserRole::STUDENT;
    int failedAttempts = 0;
    std::chrono::system_clock::time_point lockoutUntil = std::chrono::system_clock::time_point::min();
    std::string email;
    std::string fullName;
};

struct Session {
    std::string token;
    std::string username;
    UserRole role;
    std::chrono::system_clock::time_point expiresAt;
};

/**
 * @brief Authentication & Authorization Manager using PBKDF2-HMAC-SHA256.
 */
class AuthManager {
private:
    dsa::HashTable<std::string, User> users_;
    dsa::HashTable<std::string, Session> sessions_;

    static std::string sha256(const std::string& input);
    static std::string hmacSha256(const std::string& key, const std::string& msg);
    static std::string pbkdf2Sha256(const std::string& password, const std::string& salt, int iterations, int keyLength);
    static std::string generateRandomHex(size_t numBytes);
    static std::string bytesToHex(const unsigned char* data, size_t len);

public:
    AuthManager();
    ~AuthManager() = default;

    // Password Hashing & Verification
    static std::string hashPassword(const std::string& password, const std::string& salt);
    static bool verifyPassword(const std::string& password, const std::string& storedHash);

    // User Management
    bool registerUser(const std::string& username, const std::string& password,
                      UserRole role, const std::string& email, const std::string& fullName,
                      std::string& errorMsg);

    // Login & Session Generation (Lockout after 5 failed attempts)
    bool login(const std::string& username, const std::string& password,
               std::string& sessionToken, UserRole& role, std::string& errorMsg);

    // Logout & Session Invalidation
    void logout(const std::string& sessionToken);

    // Validate Session
    bool validateSession(const std::string& sessionToken, UserRole& role, std::string& username);

    // User lookup
    const User* getUser(const std::string& username) const;
};

} // namespace hostel
