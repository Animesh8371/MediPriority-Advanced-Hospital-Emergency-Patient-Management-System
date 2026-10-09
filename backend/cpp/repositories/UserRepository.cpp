#include "UserRepository.h"
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <memory>

namespace medipriority {

std::optional<User> UserRepository::findByUsername(const std::string &username) {
    auto stmt = db_.prepare(
        "SELECT user_id, username, password_hash, password_salt, role "
        "FROM users WHERE username = ?");
    stmt->setString(1, username);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());

    if (rs->next()) {
        return User(
            rs->getInt("user_id"),
            rs->getString("username"),
            rs->getString("password_hash"),
            rs->getString("password_salt"),
            rs->getString("role")
        );
    }
    return std::nullopt;
}

int UserRepository::createUser(const std::string &username, const std::string &passwordHash,
                                const std::string &passwordSalt, const std::string &role) {
    // Do not overwrite an existing user with the same username.
    if (findByUsername(username).has_value()) {
        return -1;
    }

    auto stmt = db_.prepare(
        "INSERT INTO users (username, password_hash, password_salt, role) "
        "VALUES (?, ?, ?, ?)");
    stmt->setString(1, username);
    stmt->setString(2, passwordHash);
    stmt->setString(3, passwordSalt);
    stmt->setString(4, role);
    stmt->executeUpdate();

    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

} // namespace medipriority
