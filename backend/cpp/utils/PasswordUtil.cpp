#include "PasswordUtil.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <array>

namespace medipriority {

static std::string toHex(const unsigned char *data, unsigned int len) {
    std::ostringstream oss;
    for (unsigned int i = 0; i < len; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}

std::string PasswordUtil::generateSalt() {
    unsigned char buf[16];
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        // Fall back is not cryptographically ideal, but keeps the app from
        // crashing if the RNG is briefly unavailable; logged as a warning
        // at the call site in a real deployment.
        for (auto &b : buf) b = static_cast<unsigned char>(rand() % 256);
    }
    return toHex(buf, sizeof(buf));
}

static std::string sha256Hex(const std::string &input) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(ctx, digest, &digestLen);
    EVP_MD_CTX_free(ctx);

    return toHex(digest, digestLen);
}

std::string PasswordUtil::hashPassword(const std::string &password, const std::string &salt) {
    static const int ROUNDS = 10000; // deliberately slows down brute force somewhat
    std::string current = salt + password;
    for (int i = 0; i < ROUNDS; i++) {
        current = sha256Hex(current + salt);
    }
    return current;
}

bool PasswordUtil::verifyPassword(const std::string &password, const std::string &salt,
                                   const std::string &expectedHash) {
    return hashPassword(password, salt) == expectedHash;
}

} // namespace medipriority
