#ifndef MEDIPRIORITY_PASSWORDUTIL_H
#define MEDIPRIORITY_PASSWORDUTIL_H

#include <string>

namespace medipriority {

/*
 * Password hashing for the academic MVP: SHA-256 of (salt + password),
 * with a random 16-byte hex salt generated per user, and a fixed number
 * of extra hashing rounds to slow down brute force somewhat.
 *
 * KNOWN LIMITATION (documented, not hidden): a real system should use a
 * dedicated password-hashing algorithm such as bcrypt or Argon2, which are
 * deliberately slow and memory-hard. Plain salted SHA-256 is much faster
 * to brute-force than those. We use it here to avoid pulling in an extra
 * external dependency beyond what's already required (OpenSSL, already a
 * transitive dependency of the MySQL connector), which keeps the build
 * simple for a student project. This trade-off is called out explicitly
 * in docs/architecture.md.
 */
class PasswordUtil {
public:
    static std::string generateSalt();
    static std::string hashPassword(const std::string &password, const std::string &salt);
    static bool verifyPassword(const std::string &password, const std::string &salt,
                                const std::string &expectedHash);
};

} // namespace medipriority

#endif // MEDIPRIORITY_PASSWORDUTIL_H
