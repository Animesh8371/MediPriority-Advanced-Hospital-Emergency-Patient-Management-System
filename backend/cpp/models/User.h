#ifndef MEDIPRIORITY_USER_H
#define MEDIPRIORITY_USER_H

#include <string>

namespace medipriority {

/* role: "admin" or "staff". Passwords are never stored or transmitted in
 * plaintext -- see utils/PasswordUtil for the salted-hash scheme, and
 * docs/architecture.md for why this is a simplified scheme suitable for
 * an academic prototype rather than a production auth system. */
class User {
private:
    int userId_;
    std::string username_;
    std::string passwordHash_;
    std::string passwordSalt_;
    std::string role_;

public:
    User(int userId, std::string username, std::string passwordHash,
         std::string passwordSalt, std::string role)
        : userId_(userId), username_(std::move(username)),
          passwordHash_(std::move(passwordHash)), passwordSalt_(std::move(passwordSalt)),
          role_(std::move(role)) {}

    int userId() const { return userId_; }
    const std::string &username() const { return username_; }
    const std::string &passwordHash() const { return passwordHash_; }
    const std::string &passwordSalt() const { return passwordSalt_; }
    const std::string &role() const { return role_; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_USER_H
