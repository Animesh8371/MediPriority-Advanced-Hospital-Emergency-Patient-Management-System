#ifndef MEDIPRIORITY_USERREPOSITORY_H
#define MEDIPRIORITY_USERREPOSITORY_H

#include "../utils/Database.h"
#include "../models/User.h"
#include <optional>
#include <string>

namespace medipriority {

class UserRepository {
private:
    Database &db_;

public:
    explicit UserRepository(Database &db) : db_(db) {}

    /* Returns the user if username exists, empty optional otherwise. */
    std::optional<User> findByUsername(const std::string &username);

    /* Returns the new user_id, or -1 if the username already exists. */
    int createUser(const std::string &username, const std::string &passwordHash,
                    const std::string &passwordSalt, const std::string &role);
};

} // namespace medipriority

#endif // MEDIPRIORITY_USERREPOSITORY_H
