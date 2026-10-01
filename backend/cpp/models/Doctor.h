#ifndef MEDIPRIORITY_DOCTOR_H
#define MEDIPRIORITY_DOCTOR_H

#include "Person.h"
#include <string>
#include <sstream>

namespace medipriority {

class Doctor : public Person {
private:
    std::string specialization_;
    bool available_;

public:
    Doctor(int id, std::string name, int age, std::string gender,
           std::string specialization, bool available = true)
        : Person(id, std::move(name), age, std::move(gender)),
          specialization_(std::move(specialization)),
          available_(available) {}

    const std::string &specialization() const { return specialization_; }
    bool isAvailable() const { return available_; }

    void markBusy() { available_ = false; }
    void markAvailable() { available_ = true; }

    std::string role() const override { return "Doctor"; }

    std::string describe() const override {
        std::ostringstream oss;
        oss << "Dr. " << name_ << " (" << specialization_ << ") - "
            << (available_ ? "Available" : "Busy");
        return oss.str();
    }
};

} // namespace medipriority

#endif // MEDIPRIORITY_DOCTOR_H
