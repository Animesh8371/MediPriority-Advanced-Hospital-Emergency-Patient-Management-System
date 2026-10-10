#ifndef MEDIPRIORITY_VALIDATION_H
#define MEDIPRIORITY_VALIDATION_H

#include <string>
#include <vector>

namespace medipriority {

/* Collects validation errors so callers can report ALL problems in a
 * single response, rather than one at a time. */
struct ValidationResult {
    bool valid = true;
    std::vector<std::string> errors;

    void addError(const std::string &msg) {
        valid = false;
        errors.push_back(msg);
    }
};

class Validation {
public:
    static bool isNonEmpty(const std::string &s) { return !s.empty(); }

    static bool isValidName(const std::string &name) {
        if (name.empty() || name.size() > 100) return false;
        for (char c : name) {
            if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '.' && c != '-' ) return false;
        }
        return true;
    }

    static bool isValidAge(int age) { return age > 0 && age <= 130; }

    static bool isValidPhone(const std::string &phone) {
        if (phone.size() < 7 || phone.size() > 15) return false;
        for (char c : phone) {
            if (!std::isdigit(static_cast<unsigned char>(c)) && c != '+') return false;
        }
        return true;
    }

    static bool isValidGender(const std::string &gender) {
        return gender == "Male" || gender == "Female" || gender == "Other";
    }

    static bool isValidBloodGroup(const std::string &bg) {
        static const std::vector<std::string> valid = {
            "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-", "Unknown"
        };
        for (const auto &v : valid) if (v == bg) return true;
        return false;
    }

    static bool isValidSeverity(int severity) { return severity >= 1 && severity <= 4; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_VALIDATION_H
