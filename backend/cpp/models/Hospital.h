#ifndef MEDIPRIORITY_HOSPITAL_H
#define MEDIPRIORITY_HOSPITAL_H

#include <string>

namespace medipriority {

/* A node in the SIMULATED hospital transfer network (see graph.h/dijkstra.c). */
class Hospital {
private:
    int hospitalId_;
    std::string name_;
    std::string location_; // simulated free-text location, not real GPS

public:
    Hospital(int hospitalId, std::string name, std::string location)
        : hospitalId_(hospitalId), name_(std::move(name)), location_(std::move(location)) {}

    int hospitalId() const { return hospitalId_; }
    const std::string &name() const { return name_; }
    const std::string &location() const { return location_; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_HOSPITAL_H
