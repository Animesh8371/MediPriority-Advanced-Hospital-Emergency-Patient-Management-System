#include "TriageService.h"
#include "../utils/Validation.h"
#include <ctime>

namespace medipriority {

TriageService::TriageService(EmergencyCaseRepository &repo) : repo_(repo) {
    heap_ = heap_create(16);
}

TriageService::~TriageService() {
    heap_destroy(heap_);
}

void TriageService::loadFromDatabase() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto waiting = repo_.findWaiting();
    for (const auto &ec : waiting) {
        EmergencyCase c(ec.caseId(), ec.patientId(), ec.severity(), ec.arrivalTime(), ec.category());
        ::EmergencyCase cCase;
        cCase.case_id = c.caseId();
        cCase.patient_id = c.patientId();
        cCase.severity = c.severity();
        cCase.arrival_time = c.arrivalTime();
        heap_insert(heap_, cCase);
    }
}

TriageResult TriageService::admitCase(int patientId, int severity, const std::string &category) {
    TriageResult result;

    if (!Validation::isValidSeverity(severity)) {
        result.message = "Severity must be between 1 (Critical) and 4 (Low)";
        return result;
    }
    if (category.empty()) {
        result.message = "Category is required";
        return result;
    }

    long arrivalTime = static_cast<long>(std::time(nullptr));

    EmergencyCase ec(0, patientId, severity, arrivalTime, category);
    int caseId = repo_.create(ec); // MySQL first: source of truth gets the row

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ::EmergencyCase cCase;
        cCase.case_id = caseId;
        cCase.patient_id = patientId;
        cCase.severity = severity;
        cCase.arrival_time = arrivalTime;
        heap_insert(heap_, cCase); // then the live C heap
    }

    result.success = true;
    result.caseId = caseId;
    result.message = "Emergency case admitted with priority " + severityLabel(severity);
    return result;
}

bool TriageService::peekNext(EmergencyCase &out) {
    std::lock_guard<std::mutex> lock(mutex_);
    ::EmergencyCase cCase;
    if (!heap_peek(heap_, &cCase)) return false;
    out = EmergencyCase(cCase.case_id, cCase.patient_id, cCase.severity, cCase.arrival_time, "");
    return true;
}

TriageResult TriageService::extractNext() {
    TriageResult result;
    std::lock_guard<std::mutex> lock(mutex_);

    ::EmergencyCase cCase;
    if (!heap_extract_min(heap_, &cCase)) {
        result.message = "No emergency cases waiting";
        return result;
    }

    repo_.updateStatus(cCase.case_id, "In Treatment");
    result.success = true;
    result.caseId = cCase.case_id;
    result.message = "Next case (severity " + severityLabel(cCase.severity) + ") moved to In Treatment";
    return result;
}

TriageResult TriageService::updateSeverity(int caseId, int newSeverity) {
    TriageResult result;
    if (!Validation::isValidSeverity(newSeverity)) {
        result.message = "Severity must be between 1 and 4";
        return result;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!heap_update_priority(heap_, caseId, newSeverity)) {
            result.message = "Case not found in the active priority queue";
            return result;
        }
    }

    repo_.updateSeverity(caseId, newSeverity);
    result.success = true;
    result.caseId = caseId;
    result.message = "Severity updated to " + severityLabel(newSeverity);
    return result;
}

int TriageService::waitingCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return heap_size(heap_);
}

} // namespace medipriority
