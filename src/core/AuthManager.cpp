#include "core/AuthManager.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>

namespace hostel {

namespace crypto {

// Standard 32-bit bitwise rotation
inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t gamma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t gamma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::vector<uint8_t> sha256Raw(const uint8_t* data, size_t len) {
    uint32_t H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // Pre-processing (Padding)
    size_t paddedLen = len + 1;
    while ((paddedLen % 64) != 56) {
        ++paddedLen;
    }
    paddedLen += 8;

    std::vector<uint8_t> padded(paddedLen, 0);
    if (len > 0) {
        std::memcpy(padded.data(), data, len);
    }
    padded[len] = 0x80;

    uint64_t bitLen = len * 8;
    for (int i = 0; i < 8; ++i) {
        padded[paddedLen - 1 - i] = static_cast<uint8_t>((bitLen >> (i * 8)) & 0xff);
    }

    // Process blocks
    for (size_t chunk = 0; chunk < paddedLen; chunk += 64) {
        uint32_t W[64];
        for (int t = 0; t < 16; ++t) {
            W[t] = (static_cast<uint32_t>(padded[chunk + t * 4]) << 24) |
                   (static_cast<uint32_t>(padded[chunk + t * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(padded[chunk + t * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(padded[chunk + t * 4 + 3]));
        }
        for (int t = 16; t < 64; ++t) {
            W[t] = gamma1(W[t - 2]) + W[t - 7] + gamma0(W[t - 15]) + W[t - 16];
        }

        uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
        uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

        for (int t = 0; t < 64; ++t) {
            uint32_t T1 = h + sigma1(e) + ch(e, f, g) + K[t] + W[t];
            uint32_t T2 = sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    std::vector<uint8_t> digest(32);
    for (int i = 0; i < 8; ++i) {
        digest[i * 4]     = static_cast<uint8_t>((H[i] >> 24) & 0xff);
        digest[i * 4 + 1] = static_cast<uint8_t>((H[i] >> 16) & 0xff);
        digest[i * 4 + 2] = static_cast<uint8_t>((H[i] >> 8) & 0xff);
        digest[i * 4 + 3] = static_cast<uint8_t>(H[i] & 0xff);
    }
    return digest;
}

std::vector<uint8_t> hmacSha256Raw(const uint8_t* key, size_t keyLen, const uint8_t* msg, size_t msgLen) {
    uint8_t k0[64] = {0};
    if (keyLen > 64) {
        auto h = sha256Raw(key, keyLen);
        std::memcpy(k0, h.data(), 32);
    } else {
        std::memcpy(k0, key, keyLen);
    }

    uint8_t ipad[64];
    uint8_t opad[64];
    for (int i = 0; i < 64; ++i) {
        ipad[i] = k0[i] ^ 0x36;
        opad[i] = k0[i] ^ 0x5c;
    }

    std::vector<uint8_t> innerData(64 + msgLen);
    std::memcpy(innerData.data(), ipad, 64);
    if (msgLen > 0) {
        std::memcpy(innerData.data() + 64, msg, msgLen);
    }
    auto innerHash = sha256Raw(innerData.data(), innerData.size());

    std::vector<uint8_t> outerData(64 + 32);
    std::memcpy(outerData.data(), opad, 64);
    std::memcpy(outerData.data() + 64, innerHash.data(), 32);
    return sha256Raw(outerData.data(), outerData.size());
}

} // namespace crypto

std::string AuthManager::bytesToHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string AuthManager::generateRandomHex(size_t numBytes) {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 255);

    std::vector<unsigned char> bytes(numBytes);
    for (size_t i = 0; i < numBytes; ++i) {
        bytes[i] = static_cast<unsigned char>(dis(gen));
    }
    return bytesToHex(bytes.data(), bytes.size());
}

std::string AuthManager::pbkdf2Sha256(const std::string& password, const std::string& salt, int iterations, int keyLength) {
    std::vector<uint8_t> derivedKey;
    derivedKey.reserve(keyLength);

    uint32_t blockIndex = 1;
    while (derivedKey.size() < static_cast<size_t>(keyLength)) {
        // Salt || INT_32_BE(blockIndex)
        std::vector<uint8_t> saltAndIndex(salt.begin(), salt.end());
        saltAndIndex.push_back(static_cast<uint8_t>((blockIndex >> 24) & 0xff));
        saltAndIndex.push_back(static_cast<uint8_t>((blockIndex >> 16) & 0xff));
        saltAndIndex.push_back(static_cast<uint8_t>((blockIndex >> 8) & 0xff));
        saltAndIndex.push_back(static_cast<uint8_t>(blockIndex & 0xff));

        auto uPrev = crypto::hmacSha256Raw(
            reinterpret_cast<const uint8_t*>(password.data()), password.size(),
            saltAndIndex.data(), saltAndIndex.size()
        );

        std::vector<uint8_t> uBlock = uPrev;

        for (int iter = 1; iter < iterations; ++iter) {
            uPrev = crypto::hmacSha256Raw(
                reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                uPrev.data(), uPrev.size()
            );
            for (size_t b = 0; b < 32; ++b) {
                uBlock[b] ^= uPrev[b];
            }
        }

        for (size_t b = 0; b < 32 && derivedKey.size() < static_cast<size_t>(keyLength); ++b) {
            derivedKey.push_back(uBlock[b]);
        }
        ++blockIndex;
    }

    return bytesToHex(derivedKey.data(), derivedKey.size());
}

std::string AuthManager::hashPassword(const std::string& password, const std::string& salt) {
    int iterations = 10000;
    std::string hashHex = pbkdf2Sha256(password, salt, iterations, 32);
    return "$pbkdf2-sha256$i=" + std::to_string(iterations) + "$" + salt + "$" + hashHex;
}

bool AuthManager::verifyPassword(const std::string& password, const std::string& storedHash) {
    // Format: $pbkdf2-sha256$i=10000$salt$hash
    if (storedHash.rfind("$pbkdf2-sha256$", 0) != 0) return false;

    size_t iterStart = storedHash.find("$i=") + 3;
    size_t iterEnd = storedHash.find('$', iterStart);
    if (iterStart == std::string::npos || iterEnd == std::string::npos) return false;
    int iterations = std::stoi(storedHash.substr(iterStart, iterEnd - iterStart));

    size_t saltEnd = storedHash.find('$', iterEnd + 1);
    if (saltEnd == std::string::npos) return false;
    std::string salt = storedHash.substr(iterEnd + 1, saltEnd - (iterEnd + 1));
    std::string expectedHash = storedHash.substr(saltEnd + 1);

    std::string computedHash = pbkdf2Sha256(password, salt, iterations, 32);
    return computedHash == expectedHash;
}

AuthManager::AuthManager() {
    // Seed default administrator account: admin / Admin@123
    std::string err;
    registerUser("admin", "Admin@123", UserRole::ADMIN, "admin@hostel.edu", "System Administrator", err);
    // Seed sample warden, student, and guard accounts
    registerUser("warden1", "Warden@123", UserRole::WARDEN, "warden@hostel.edu", "Chief Warden Sharma", err);
    registerUser("student1", "Student@123", UserRole::STUDENT, "student1@hostel.edu", "Aarav Sharma", err);
    registerUser("guard1", "Guard@123", UserRole::GUARD, "guard@hostel.edu", "Main Gate Guard", err);
}

bool AuthManager::registerUser(const std::string& username, const std::string& password,
                              UserRole role, const std::string& email, const std::string& fullName,
                              std::string& errorMsg) {
    if (username.empty() || password.empty()) {
        errorMsg = "Username and password cannot be empty.";
        return false;
    }
    if (users_.contains(username)) {
        errorMsg = "Username '" + username + "' is already taken.";
        return false;
    }
    if (password.length() < 6) {
        errorMsg = "Password must be at least 6 characters long.";
        return false;
    }

    std::string salt = generateRandomHex(16);
    std::string hash = hashPassword(password, salt);

    User u;
    u.username = username;
    u.passwordHash = hash;
    u.salt = salt;
    u.role = role;
    u.email = email;
    u.fullName = fullName;
    u.failedAttempts = 0;
    u.lockoutUntil = std::chrono::system_clock::time_point::min();

    users_.insert(username, u);
    return true;
}

bool AuthManager::login(const std::string& username, const std::string& password,
                        std::string& sessionToken, UserRole& role, std::string& errorMsg) {
    User* user = users_.find(username);
    if (!user) {
        errorMsg = "Invalid username or password.";
        return false;
    }

    auto now = std::chrono::system_clock::now();
    if (now < user->lockoutUntil) {
        auto remainingSec = std::chrono::duration_cast<std::chrono::seconds>(user->lockoutUntil - now).count();
        errorMsg = "Account temporarily locked due to 5 failed attempts. Try again in " +
                   std::to_string(remainingSec) + " seconds.";
        return false;
    }

    if (!verifyPassword(password, user->passwordHash)) {
        ++user->failedAttempts;
        if (user->failedAttempts >= 5) {
            user->lockoutUntil = now + std::chrono::minutes(15);
            errorMsg = "Account locked for 15 minutes due to 5 consecutive failed login attempts.";
        } else {
            errorMsg = "Invalid username or password. (" + std::to_string(5 - user->failedAttempts) + " attempts remaining)";
        }
        return false;
    }

    // Login successful
    user->failedAttempts = 0;
    user->lockoutUntil = std::chrono::system_clock::time_point::min();

    sessionToken = generateRandomHex(24);
    role = user->role;

    Session sess;
    sess.token = sessionToken;
    sess.username = username;
    sess.role = role;
    sess.expiresAt = now + std::chrono::hours(24);

    sessions_.insert(sessionToken, sess);
    return true;
}

void AuthManager::logout(const std::string& sessionToken) {
    sessions_.erase(sessionToken);
}

bool AuthManager::validateSession(const std::string& sessionToken, UserRole& role, std::string& username) {
    Session* sess = sessions_.find(sessionToken);
    if (!sess) return false;

    if (std::chrono::system_clock::now() > sess->expiresAt) {
        sessions_.erase(sessionToken);
        return false;
    }

    role = sess->role;
    username = sess->username;
    return true;
}

const User* AuthManager::getUser(const std::string& username) const {
    return users_.find(username);
}

} // namespace hostel
