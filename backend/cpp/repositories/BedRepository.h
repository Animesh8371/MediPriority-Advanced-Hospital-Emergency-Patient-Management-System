#ifndef MEDIPRIORITY_BEDREPOSITORY_H
#define MEDIPRIORITY_BEDREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Bed.h"
#include <optional>
#include <vector>

namespace medipriority {

struct BedActionResult {
    bool success = false;
    std::string message;
};

class BedRepository {
private:
    Database &db_;
public:
    explicit BedRepository(Database &db) : db_(db) {}

    std::optional<Bed> findById(int bedId);
    std::vector<Bed> findAll();
    std::vector<Bed> findAvailable(const std::string &bedType);
    int create(const std::string &bedType);

    /* Allocates a bed to a patient INSIDE A TRANSACTION: re-checks the bed
     * is still unoccupied at the moment of allocation (guards against a
     * race where two requests try to allocate the same bed), then marks it
     * occupied and records a patient_assignments row. Returns success=false
     * with a message if the bed was already occupied or does not exist --
     * it never silently allocates an already-occupied bed. */
    BedActionResult allocate(int bedId, int patientId, int doctorId);

    /* Releases a bed and closes out the matching patient_assignments row. */
    BedActionResult release(int bedId);
};

} // namespace medipriority
#endif // MEDIPRIORITY_BEDREPOSITORY_H
