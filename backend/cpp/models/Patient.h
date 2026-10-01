#ifndef MEDIPRIORITY_PATIENT_H
#define MEDIPRIORITY_PATIENT_H

#include "Person.h"
#include <string>
#include <sstream>

namespace medipriority {

/* Blood group is restricted to a fixed set of simulated values;
 * validated by services/Validation, not enforced at this layer. */
class Patient : public Person {
private:
    std::string phone_;
    std::string bloodGroup_;
    std::string emergencyContact_;
    std::string dateOfBirth_; // "YYYY-MM-DD", stored as string for MVP simplicity

public:
    Patient(int id, std::string name, int age, std::string gender,
            std::string phone, std::string bloodGroup,
            std::string emergencyContact, std::string dateOfBirth)
        : Person(id, std::move(name), age, std::move(gender)),
          phone_(std::move(phone)),
          bloodGroup_(std::move(bloodGroup)),
          emergencyContact_(std::move(emergencyContact)),
          dateOfBirth_(std::move(dateOfBirth)) {}

    const std::string &phone() const { return phone_; }
    const std::string &bloodGroup() const { return bloodGroup_; }
    const std::string &emergencyContact() const { return emergencyContact_; }
    const std::string &dateOfBirth() const { return dateOfBirth_; }

    std::string role() const override { return "Patient"; }

    std::string describe() const override {
        std::ostringstream oss;
        oss << "Patient #" << id_ << " " << name_ << " (" << age_ << ", " << gender_
            << ", blood group " << bloodGroup_ << ")";
        return oss.str();
    }
};

} // namespace medipriority

#endif // MEDIPRIORITY_PATIENT_H
