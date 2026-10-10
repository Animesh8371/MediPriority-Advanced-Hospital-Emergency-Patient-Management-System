#ifndef MEDIPRIORITY_DOCTOR_H
#define MEDIPRIORITY_DOCTOR_H

#include "Person.h"
#include <string>
#include <sstream>

namespace medipriority {

class Doctor : public Person {
private:
    std::string specialization_;
    bool available_;      // on-duty flag (manual "mark busy / mark free")
    int maxPatients_;     // how many patients this doctor can handle at once
    int currentPatients_; // active (unreleased) assignments, computed by SQL

public:
    Doctor(int id, std::string name, int age, std::string gender,
           std::string specialization, bool available = true,
           int maxPatients = 5, int currentPatients = 0)
        : Person(id, std::move(name), age, std::move(gender)),
          specialization_(std::move(specialization)),
          available_(available),
          maxPatients_(maxPatients),
          currentPatients_(currentPatients) {}

    const std::string &specialization() const { return specialization_; }
    bool isAvailable() const { return available_; }
    int maxPatients() const { return maxPatients_; }
    int currentPatients() const { return currentPatients_; }
    int freeSlots() const { return maxPatients_ > currentPatients_ ? maxPatients_ - currentPatients_ : 0; }
    /* A doctor can take another patient only if on duty AND below capacity. */
    bool acceptingPatients() const { return available_ && currentPatients_ < maxPatients_; }

    void markBusy() { available_ = false; }
    void markAvailable() { available_ = true; }

    std::string role() const override { return "Doctor"; }

    std::string describe() const override {
        std::ostringstream oss;
        oss << "Dr. " << name_ << " (" << specialization_ << ") - "
            << (available_ ? "Available" : "Busy")
            << " [" << currentPatients_ << "/" << maxPatients_ << " patients]";
        return oss.str();
    }
};

} // namespace medipriority

#endif // MEDIPRIORITY_DOCTOR_H
