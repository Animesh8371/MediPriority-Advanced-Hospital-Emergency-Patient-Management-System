#ifndef MEDIPRIORITY_BED_H
#define MEDIPRIORITY_BED_H

#include <string>

namespace medipriority {

/* bedType: "General", "Emergency", "ICU" */
class Bed {
private:
    int bedId_;
    std::string bedType_;
    bool occupied_;
    int occupiedByPatientId_; // 0 if unoccupied

public:
    Bed(int bedId, std::string bedType, bool occupied = false, int occupiedByPatientId = 0)
        : bedId_(bedId), bedType_(std::move(bedType)), occupied_(occupied),
          occupiedByPatientId_(occupiedByPatientId) {}

    int bedId() const { return bedId_; }
    const std::string &bedType() const { return bedType_; }
    bool isOccupied() const { return occupied_; }
    int occupiedByPatientId() const { return occupiedByPatientId_; }

    void allocate(int patientId) { occupied_ = true; occupiedByPatientId_ = patientId; }
    void release() { occupied_ = false; occupiedByPatientId_ = 0; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_BED_H
