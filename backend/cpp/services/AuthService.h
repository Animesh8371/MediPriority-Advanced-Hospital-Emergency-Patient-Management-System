#ifndef MEDIPRIORITY_AUTHSERVICE_H
#define MEDIPRIORITY_AUTHSERVICE_H

#include "../repositories/UserRepository.h"
#include <string>
#include <unordered_map>

namespace medipriority {

struct LoginResult {
    bool success = false;
    std::string message;
    int userId = -1;
    std::string role;
    std::string token; // opaque session token (see AuthService::generateToken)
};

struct RegisterResult {
    bool success = false;
    std::string message;
    int userId = -1;
};

/*
 * Session tokens: a random 32-byte hex string kept in an in-memory map
 * (token -> user_id) inside this service instance. This is a SIMPLIFIED
 * scheme suitable for a single-process academic prototype -- a real
 * deployment would use signed/expiring JWTs or a shared session store
 * (Redis, DB-backed sessions) so sessions survive a server restart and
 * work across multiple server processes. Documented as a known
 * limitation in docs/architecture.md.
 */
class AuthService {
private:
    UserRepository &userRepo_;
    std::unordered_map<std::string, int> activeSessions_; // token -> user_id
    std::unordered_map<std::string, std::string> sessionRoles_; // token -> role

    std::string generateToken();

public:
    explicit AuthService(UserRepository &userRepo) : userRepo_(userRepo) {}

    RegisterResult registerUser(const std::string &username, const std::string &password,
                                 const std::string &role);
    LoginResult login(const std::string &username, const std::string &password);

    /* Returns the user_id if token is a valid active session, else -1. */
    int validateToken(const std::string &token);
    std::string roleForToken(const std::string &token);
    void logout(const std::string &token);
};

} // namespace medipriority

#endif // MEDIPRIORITY_AUTHSERVICE_H
