#ifndef MEDIPRIORITY_AMBULANCE_H
#define MEDIPRIORITY_AMBULANCE_H

#include <string>

namespace medipriority {

class Ambulance {
private:
    int ambulanceId_;
    std::string vehicleNumber_;
    std::string status_; // "Available", "Assigned", "Maintenance"

public:
    Ambulance(int ambulanceId, std::string vehicleNumber, std::string status = "Available")
        : ambulanceId_(ambulanceId), vehicleNumber_(std::move(vehicleNumber)),
          status_(std::move(status)) {}

    int ambulanceId() const { return ambulanceId_; }
    const std::string &vehicleNumber() const { return vehicleNumber_; }
    const std::string &status() const { return status_; }

    bool isAvailable() const { return status_ == "Available"; }
    void assign() { status_ = "Assigned"; }
    void release() { status_ = "Available"; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_AMBULANCE_H
