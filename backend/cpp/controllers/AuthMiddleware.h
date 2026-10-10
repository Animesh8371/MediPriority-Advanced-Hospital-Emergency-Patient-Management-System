#ifndef MEDIPRIORITY_AUTHMIDDLEWARE_H
#define MEDIPRIORITY_AUTHMIDDLEWARE_H

#include "crow_all.h"
#include "../services/AuthService.h"
#include <optional>
#include <string>

namespace medipriority {

/* Extracts "Authorization: Bearer <token>" and validates it against the
 * AuthService's active sessions. Returns the user_id if valid, else
 * std::nullopt. Used at the top of every protected route handler. */
inline std::optional<int> authenticate(const crow::request &req, AuthService &authService) {
    std::string header = req.get_header_value("Authorization");
    const std::string prefix = "Bearer ";
    if (header.size() <= prefix.size() || header.compare(0, prefix.size(), prefix) != 0) {
        return std::nullopt;
    }
    std::string token = header.substr(prefix.size());
    int userId = authService.validateToken(token);
    if (userId < 0) return std::nullopt;
    return userId;
}

} // namespace medipriority

#endif // MEDIPRIORITY_AUTHMIDDLEWARE_H
