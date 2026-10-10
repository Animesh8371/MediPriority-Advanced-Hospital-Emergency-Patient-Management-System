#include "AuthService.h"
#include "../utils/PasswordUtil.h"
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>

namespace medipriority {

std::string AuthService::generateToken() {
    unsigned char buf[32];
    RAND_bytes(buf, sizeof(buf));
    std::ostringstream oss;
    for (unsigned char b : buf) oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

RegisterResult AuthService::registerUser(const std::string &username, const std::string &password,
                                          const std::string &role) {
    RegisterResult result;

    if (username.empty() || password.size() < 6) {
        result.message = "Username required and password must be at least 6 characters";
        return result;
    }
    if (role != "admin" && role != "staff") {
        result.message = "Role must be 'admin' or 'staff'";
        return result;
    }

    std::string salt = PasswordUtil::generateSalt();
    std::string hash = PasswordUtil::hashPassword(password, salt);

    int userId = userRepo_.createUser(username, hash, salt, role);
    if (userId < 0) {
        result.message = "Username already exists";
        return result;
    }

    result.success = true;
    result.userId = userId;
    result.message = "User registered successfully";
    return result;
}

LoginResult AuthService::login(const std::string &username, const std::string &password) {
    LoginResult result;

    auto userOpt = userRepo_.findByUsername(username);
    if (!userOpt.has_value()) {
        result.message = "Invalid username or password";
        return result;
    }

    const User &user = userOpt.value();
    if (!PasswordUtil::verifyPassword(password, user.passwordSalt(), user.passwordHash())) {
        result.message = "Invalid username or password";
        return result;
    }

    std::string token = generateToken();
    activeSessions_[token] = user.userId();
    sessionRoles_[token] = user.role();

    result.success = true;
    result.userId = user.userId();
    result.role = user.role();
    result.token = token;
    result.message = "Login successful";
    return result;
}

int AuthService::validateToken(const std::string &token) {
    auto it = activeSessions_.find(token);
    return it != activeSessions_.end() ? it->second : -1;
}

std::string AuthService::roleForToken(const std::string &token) {
    auto it = sessionRoles_.find(token);
    return it != sessionRoles_.end() ? it->second : "";
}

void AuthService::logout(const std::string &token) {
    activeSessions_.erase(token);
    sessionRoles_.erase(token);
}

} // namespace medipriority
